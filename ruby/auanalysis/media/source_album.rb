require 'track_boundary_analysis'
require 'media/album'
require 'media/source_file'
require 'media/source_file_audacity'
require 'media/source_track'
require 'media/metadata_provider/discogs'
require 'std/ConfigYAML'
require 'std/module_logging'
require 'std/ObjectHost'
require 'std/raise'
require 'std/SafeThread'
require 'std/UniqueIDConfig'
require 'date'

# Source recording of an album that can be processed to create modified, format-shifted versions.
class SourceAlbum < Album
  include ConfigYAML
  include ModuleLogging
  include Raise
  include ObjectHost
  include UniqueIDConfig
  include Viewable
  
  class ExportData
    attr_accessor :output_format
    attr_accessor :export_path
  end
  
  attr_accessor :export_data # only set during dest_path
  
  persistent_infer :comment do ''; end
  
  # An alternate unique ID -- fix this, should rename variable.
  persistent_accessor :id_album
  
  persistent_infer :sourcetrack_discovery_paths do
    if library()
      library().source_paths()
    else
      []
    end
  end
  
  persistent_infer :dest_dir_transform do
    $app.exporter.default_dir_transform
  end

  persistent_accessor :purchase_date
  persistent_accessor :purchase_price
  persistent_accessor :source_files
  persistent_accessor :source_tracks

  default :id_album, ''
  default :purchase_date, Date.today
  default :purchase_price, 19.50
  default :source_files, [SourceFileAudacity.new]
  default :source_tracks do [SourceTrack.new(self, nil, '')] ;end
  
  host :source_files
  host :source_tracks
  
  def self.config_version
    2
  end
  
  def self.config_upgrade(new, old, old_version)
    if old_version < 1
      new.purchase_date = Date.today
    elsif old_version < 2
      if old.id_album.class == Path
        # bad code meant that id_album got saved as a path with incorrect is_directory attribute
        oldpath = old.id_album
        oldpath.instance_variable_set(:@is_directory, true)
        new.id_album = old.id_album.literal
      end
    end
  end
  
  def initialize(library)
    super(library)
    @processed = false
    @library = library 
    symboldefaults_init(SourceAlbum)
  end
  
  def dest_path(output_format, artist_name=nil, export_base_path=nil, dest_dir_transform=dest_dir_transform())
    subject = dup()
    
    subject.export_data = ExportData.new
    subject.export_data.output_format = output_format
    subject.export_data.export_path = if export_base_path
      export_base_path
    else
      output_format.export_base_path(library().dest_path)
    end
    
    if artist_name
      subject.artist = library().artists.add_or_retrieve(artist_name) # tbd: make artist temporary only
    end
    
    SaferEval.path_directory(subject, dest_dir_transform)
  end
  
  # Try and calculate the list of SourceFiles that composes this SourceAlbum 
  def extract_source_files(path)
    result = []
    
    SourceFile.subclasses.each do |source_file_class|
      result += path.with_file_replace("*.#{source_file_class.extension}").ls.sort.collect do |p|
        SourceFile.new_for_path(self, p)
      end
    end
    
    result
  end

  # Try and calculate a list of SourceTracks from the information in the SourceAlbum.
  def extract_source_tracks()
    source_tracks = []
    
    source_files().each do |sf|
      begin 
        source_tracks.concat(sf.extract_source_tracks)
      rescue SourceFile::ExtractTracksException => e
        log { "Track extraction failed: #{e}" }
      end
    end
    
    source_tracks.sort { |x, y| x.description <=> y.description }
  end
  
  # Try and calculate a list of Tracks belonging to the album this object references.
  # The given dir transforms are applied to the source album data to try and find previously exported
  # files.
  # Source tracks must be present. Only a number of tracks equal to the source track length are extracted.
  # Files with extensions matching those used by the supplied output formats are checked.
  def extract_album_tracks(artist_name = nil, export_paths = nil, dest_dir_transforms = nil, output_formats = nil)
    raise "No source tracks defined." if source_tracks().empty?
    raise "No library is defined, and no export paths were supplied." if !export_paths && !library()
    
    implicit_export_paths = [library().dest_path()]
    export_paths ||= implicit_export_paths + sourcetrack_discovery_paths()
    dest_dir_transforms ||= [dest_dir_transform()]
    exporters ||= OutputFormat.subclasses
    
    log { "Source album '#{title()}': extracting track information." }
    
    source_file_collections = []
      
    dest_dir_transforms.each do |transform|
      export_paths.each do |export_path|
        exporters.each do |exporter|
          search_path = dest_path(exporter, artist_name, export_path)
          
          log { "  Searching #{search_path}..." }
          result = search_path.ls("*.#{exporter.output_extension}")
          result.each{|path| log { "    Found #{path}" }}
          source_file_collections << result if !result.empty?
        end
      end
    end
    
    # sort -- put arrays with newest files first
    source_file_collections.sort_by { |array| array.collect{ |path| path.mtime }.max } 

    # choose newer collections of files with a number of exported files greater than or equal to the
    # number of source tracks. 
    tracks = []
      
    source_file_collections.each do |dest_files|
      if dest_files.length >= source_tracks().length
        dest_files.each do |path|
          begin
            log { "  from #{path}: " }
              
            tracks << Track.new_from_path(self, path)
            
            if source_tracks().length >= tracks.length
              source_track_link = source_tracks()[tracks.length - 1]
              
              log { "    #{tracks.last.title}: link to source track '#{source_track_link.description}'" }
        
              source_track_link.track = tracks.last
              source_track_link.sync_with_dest
            end 
          rescue Track::Exception => e
            log { "  extraction failed: #{e}" }
          end
        end
        
        break
      end
    end

    if tracks.empty?
      log { "  No processed files found at destination. Attempting to infer track information from source files." }
      # No processed files were found -- add a track for each source track present
      # tbd: detect multiple recordings of the same album "A, B, 2_A, 2_B"
      source_tracks.each do |source_track|
        begin
          source_file_path = source_track.source_file.path
          
          log { "  from #{source_file_path}: " }
          
          tracks << Track.new(self)
          tracks.last.title = "#{source_file_path.file}:#{source_track.description}" if !tracks.last.title
          
          log { "    #{tracks.last.title}: inferred from source track" }

          source_track.track = tracks.last
        end
      end
    end
    
    if tracks.empty?
      log { "  No tracks extracted." }
    end
    
    tracks
  end
  
  def outdated_tracks(output_formats)
    result = output_formats.collect do |output_format|
      source_tracks().find_all { |t| t.outdated?(output_format) }
    end
    
    result.flatten!
  end 
  
  def outdated?(output_formats)
    source_tracks().empty? || !outdated_tracks(output_formats).empty?
  end
  
  def set_from(album)
    super
    
    source_tracks().zip(tracks()) do |source_track, track|
      source_track.track = track
    end
  end
  
  def set_tracks_from(album)
    clear_tracks()
    
    album.tracks.each_with_index do |track, i|
      new_track = Track.new(self)
      new_track.set_from(track)
      source_tracks[i].track = new_track if i < source_tracks.length
    end
  end

  def sort_tracks
    super
    
    @source_tracks.replace(@source_tracks.sort_by do |source_track|
      index = @tracks.index(source_track.track)
      if index == nil
        source_tracks.length
      else
        index
      end 
    end)
  end
  
  # returns a list of paths designating where the source data for this album can be found.
  def source_paths_existing(base_paths)
    result = []
      
    base_paths.each do |base_path|
      path = source_path(base_path)
      result << path if path.exists?
    end
    
    result
  end
  
  def source_path(library_base_path)
    library_base_path + id_album()
  end
end

