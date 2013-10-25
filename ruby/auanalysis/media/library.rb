require 'media/artist'
require 'media/album'
require 'media/format'
require 'media/source_album'
require 'media/source_file'
require 'std/view/array_records'
require 'cgi'
require 'std/progress'
require 'std/module_logging'
require 'std/ObjectHost'
require 'std/unique'

# Data also used by library that can be overridden on a per-machine basis.
# Separated into a different object so that a separate config file will be used. Is this ok?
# Should user/interface requirements dictate implementation like this? 
class LibraryLocalOverrides
  include ConfigYAML
  include SymbolDefaults
  include Viewable
  
  persistent_infer :source_paths, nil
  attr_model_value :source_paths, [Path.new("c:/path")]
  
  persistent_infer :exporter_dest_path, nil
  attr_model_value :exporter_dest_path, Path.new("c:/path")

  symboldefaults_init()

  def configElementName
    self.class.name
  end
end

class Library
  include ConfigYAML
  include ModuleLogging
  include ObjectHost
  include Raise
  include UniqueIDConfig
  include UniqueHost
  include Viewable
  
  unique_id_registry Artist, :artists
  unique_id_registry Format, :formats
  persistent_accessor :source_albums
  persistent :source_paths

  host :source_albums
  
  default_noinit :dest_path, Path.new
  default_noinit :source_paths, [Path.new]
  default_noinit :source_albums do [SourceAlbum.new(self)]; end
    
  default_view :source_albums, 'ViewArrayRecords'
  
  def self.config_version
    1
  end
  
  def self.config_upgrade(new, old, old_version)
    if old_version < 1
      raise "All data should be converted already."
      new.source_paths = [old.instance_variable_get(:@source_path)]
    end
  end

  # Returns a library used in a transient way, meaning that it and its data will be discarded after a short period of operation.
  def self.new_transient
    self.new(nil)
  end
    
  def initialize(local_library_overrides, source_paths = [Path.new], dest_path = Path.new)
    symboldefaults_init(Library)
    
    @local_library_overrides = local_library_overrides
    @source_albums = []
    @source_paths = source_paths
    @dest_path = dest_path
    
    Format.new('12" vinyl', self) 
    Format.new('10" vinyl', self) 
    Format.new('7" vinyl', self)
    Format.new('CD', self)
    Format.new('5" CD', self)
    Format.new('MP3', self)
    Format.new('WAV', self)
    Format.new('Flac', self)
    Format.new('Unknown/Other', self)
  end
  
  def self.is_acronym?(word)
    word.match(/\W/)
  end
  
  def self.title_capitalisation_exceptions 
    ['DJ']
  end
  
  # Returns nil if no exception is available  
  def self.title_capitalisation_exception_for_word(word)
    title_capitalisation_exceptions().each do |cap_except|
      if word.downcase == cap_except.downcase
        return cap_except
      end
    end
    
    nil
  end
  
  # Returns a "clean" version of a title-like string, such as album title or artist name.
  def self.cleaned_title(title)
    title.gsub(/([\w.-]+)/) do |word|
      replacement = if is_acronym?(word)
        word
      else
        title_capitalisation_exception_for_word(word)
      end
      
      replacement ||= word.capitalize
    end
  end
  
  def self.cleaned_artist_name(artist_name)
    cleaned_title(artist_name)
  end

  def configElementName
    self.class.name
  end
  
  # Attempt to determine and add data for a SourceAlbum from the given path, and also add an Album
  # connected to that SourceAlbum.
  def add_source_data_from_path(path)
    source_album = create_source_data_from_path(path)
    @source_albums << source_album
    @source_albums_hash_id_album ||= {}
    @source_albums_hash_id_album[source_album.id_album.downcase] = source_album
  end
  
  def source_albums_hash_id_album_downcase
    if !@source_albums_hash_id_album
      @source_albums_hash_id_album = {}
      @source_albums.each do |source_album|
        @source_albums_hash_id_album[source_album.id_album.downcase] = source_album
      end
    end
    
    @source_albums_hash_id_album 
  end
  
  def has_album_with_relative_path?(path)
    source_albums_hash_id_album_downcase().has_key?(path.literal.downcase)
  end

  def dest_path
    if @local_library_overrides.exporter_dest_path
      @local_library_overrides.exporter_dest_path
    else
      @dest_path
    end
  end
  
  def source_paths
    if @local_library_overrides.source_paths
      @local_library_overrides.source_paths
    else
      @source_paths
    end
  end
    
  def path_is_valid_album?(path)
    path.directory? && path_has_valid_source_files?(path)
  end
  
  def path_has_valid_source_files?(path)
    found = valid_source_extensions().detect do |s| 
      !(path.with_file("*.#{s}")).ls.empty? 
    end
    
    found != nil
  end
  
  def reimport(source_album)
    index = @source_albums.index(source_album)
    
    if index == nil
      raise "Source album '#{source_album}' is not a record of this library."
    end
    
    source_paths = source_album.source_paths_existing(source_paths())
    raise "Source album '#{source_album}' exists in multiple source paths: #{source_paths.join($/)}" if source_paths.length != 1
    
    new_album = create_source_data_from_path(source_paths.first)
    new_album.set_tracks_from(source_album)
    
    @source_albums.replace(@source_albums[0...index] + [new_album] + Array(@source_albums[(index+1)..-1]))
  end
  
  def to_s
    "Music library sourced from #{source_paths().join(', ')}"
  end
  
  def update(max_items = nil, match_by_relative_paths = false)
    Progress.each(source_paths(), 'Build source libraries') do |source_path|
      candidates = (source_path + Path.new('*/*')).ls + (source_path.with_file_replace('*')).ls
      candidates.delete_if { |e| !path_is_valid_album?(e) }
      
      Progress.each(candidates, "Build source library #{source_path}") do |e|
        break if max_items && source_albums().length == max_items  
        
        if e.directory?
          if !has_album_with_relative_path?(e.relative(source_path))
            log { "Source data at #{e.relative(source_path)} does not yet exist in library, importing..." }
            add_source_data_from_path(e)
          end
        end
      end
    end
  end
  
  def valid_source_extensions
    SourceFile.subclasses.collect { |c| c.extension }
  end

  def viewable_dirtyTestReference
    nil # tbd: standard method is too costly, investigate
  end
  
protected
  def create_source_data_from_path(path)
    source_path = nil
    
    # determine which of the library's source paths is parent to the input path
    source_paths().each do |each_source_path|
      Algorithm.recurse(path, :parent) do |parent_path|
        if each_source_path == parent_path
          source_path = each_source_path
          break
        end
      end
    end
    
    raise "Path #{path} was not contained by one of the library's source paths: #{source_paths().join($/)}" if !source_path
    
    new_source_album = SourceAlbum.new(self)
    
    new_source_album.title = path.relative(path.parent).literal.chomp('/')
    
    artist_name = if path.parent != source_path
      CGI::unescape(path.parent.relative(source_path).literal.chomp('/'))
    else
      new_source_album.artist = Artist.various(self)
      nil
    end
  
    new_source_album._library = self 
    
    new_source_album.id_album = path.relative(source_path).literal
    log { "  Finding source files." }
    new_source_album.source_files = new_source_album.extract_source_files(path)
    
    warn { "    No source files found, this could be an error." } if new_source_album.source_files.empty?

    log { "  Finding source track data where possible." }
    new_source_album.source_tracks = new_source_album.extract_source_tracks
    new_source_album.sort_tracks
    
    log { "  Finding track information where possible." }
    begin
      new_source_album.tracks = new_source_album.extract_album_tracks(artist_name)
      
      begin
        new_source_album.infer_album_artist_from_track_data
      rescue SourceAlbum::Exception
      end
    rescue SourceAlbum::Exception => e
      log { "Error extracting tracks from #{path}, skipping (#{e})" }
    end
    
    if !new_source_album.artist && artist_name
      new_source_album.artist = artists.add_or_retrieve(artist_name)
    end
    
    new_source_album
  end
end

