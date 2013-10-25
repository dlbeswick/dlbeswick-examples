require 'std/raise'
require 'media/library'
require 'media/source_file'
require 'transform/exporter'
require 'std/view/user_actions'
require 'std/view/Viewable'

module SaferEval
  def self.eval(instance, string)
    begin
      SafeThread.new do
        $SAFE = 4
        instance.instance_eval(string)
      end.value.untaint
    rescue ::Exception=>e
      $stderr.puts "SAFEREVAL EXCEPTION:"
      $stderr.puts string
      $stderr.puts
      $stderr.puts e
      $stderr.puts e.message
      $stderr.puts e.backtrace
    end
  end
  
  def self.path(instance, string)
    result = eval(instance, string)
    Path.new(result)
  end
  
  def self.path_directory(instance, string)
    result = eval(instance, string)
    Path.new(result, true)
  end
  
  def self.string(instance, string)
    eval(instance, "\"#{string.gsub("\"", "\\\"")}\"")
  end
end

class App  
  include ConfigYAML
  include SymbolDefaults
  include Viewable
  
  persistent_accessor :exporter
  attr_reader :library_local_overrides
  attr_reader :library
  
  attr_reader :test_data_path
  
  # This is nil unless test mode is active.
  attr_reader :test_library
  
  default :exporter, Exporter.new
  
  default_view :exporter, "Views::AttributeObjectReferenceDescription"
  default_view :library, "Views::AttributeObjectReferenceDescription"
  default_view :test_library, "Views::AttributeObjectReferenceDescription"
  
  def initialize
    symboldefaults_init(App)
    
    @test_data_path ||= Path.new("test").absolute
  end
  
  def load
    super if config_exists?

    always_rebuild = false
    max_tracks = 2
    need_rebuild = always_rebuild == true

    @library_local_overrides = begin
      LibraryLocalOverrides.load(nil, nil, false)
    rescue DLB::Config::ExceptionLoad => e
      puts "Error loading local library overrides, creating new instance: #{e}\n\nBacktrace:\n#{e.backtrace.join("\n")}"
      LibraryLocalOverrides.new
    end
    
    @library = Library.new(
      @library_local_overrides,
      [Path.new('d:/data/recordings'), Path.new('d:/data/source-only recordings')], 
      Path.new('d:/data/mp3')
    )
    
    if !always_rebuild
      begin
        @library.load
        UniqueIDConfig.ensure_resolution(@library)
        @library.update(max_tracks)
      rescue DLB::Config::ExceptionLoad=>e
        puts "Error loading library: #{e}\n\nBacktrace:\n#{e.backtrace.join("\n")}"
        need_rebuild = true
      end
    end
    
    if need_rebuild
      puts "Couldn't load, building library."
      @library.add_source_data_from_path(Path.new('d:/data/recordings/Natalie Cole/Good To Be Back'))
      @library.update(max_tracks)
    end

    if test_mode?
      @test_library = Library.new(
        LibraryLocalOverrides.new,
        [test_library_import_path()],
        test_library_export_path
      )
      
      def @test_library.configElementName
        "test_library"
      end
      
      @test_library.update
    end
  end
  
  # Test mode is activated with the --test command line switch.
  def test_mode?
    $options[:test] == true
  end
  
  def test_output_path
    test_data_path() + Path.new("output/")
  end
  
  def test_library_import_path
    test_data_path() + Path.new("for-library-import/")
  end

  def test_library_export_path
    test_output_path() + Path.new("library_export/")
  end
end

class AppView < Views::UserActionsButtons
  suits_class(App)
  
  def save_settings
    target().save
    target().library_local_overrides.save
  end
  
  def local_settings
    ViewContainers::Single.new(target().library_local_overrides, self, View)
  end
  
  def converter
    chosen_path = GUI.dir_browser(target().library.source_paths.first.to_sys, "Choose path:")
    
    if chosen_path
      temp_library_path = chosen_path.parent
      
      library = Library.new([temp_library_path], target().library.dest_path)
      
      album_dirs = chosen_path.ls("**/*")
      album_dirs.delete_if { |path| !path.directory? }
      album_dirs = [chosen_path] if album_dirs.empty? # this will be the case if a 'various artists' path was chosen
        
      album_dirs.each do |path|
        library.add_source_data_from_path(path)
      end
      
      ViewContainers::Single.new(library, self)
    end
  end
  
  class X
    def self.next(what, parent)
      Wx::Timer.after(1000) do
        record_view = what.view.target_view.view_for_symbol(:source_albums)
        index = record_view.index + 1
        index = 0 if index >= record_view.target.length
        record_view.view_index(index)
        self.next(what, parent)
      end
    end
  end
  
  def test_gui_crash
    x = ViewContainers::Single.new(target.library, self)
    X.next(x, parent)
  end
  
  def test_wavelet_thresholding
    require "test/test_track_boundaries"
    ViewContainers::Single.new(ThresholdWaveletTest.new, self)
  end
  
  def test_track_boundaries
    require "test/test_track_boundaries"
    ViewContainers::Single.new(TrackBoundariesTest.new, self)
  end

  def test_wavelet
    require "test/test_track_boundaries"
    ViewContainers::Single.new(WaveletTest.new, self)
  end
end
