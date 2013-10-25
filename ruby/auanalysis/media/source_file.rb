require 'media/source_track'

# Multiple source recordings comprising a recording of an album.
# A single SourceFile may contain recordings of many tracks, and a SourceAlbum can be composed of
# multiple SourceFiles.
class SourceFile
  include Abstract
  include ConfigYAML
  include EnumerableSubclasses
  include Raise
  include UniqueIDConfig
  include Viewable
  
  persistent_accessor :source_album
  persistent_accessor :path
  persistent_accessor :transient
  
  default :path, Path.new
  default :transient, false

  class ExtractTracksException < Exception
  end

  def self.new_for_path?(source_path)
    subclasses().each do |c|
      if Path.extension_eql?(c.extension, source_path)
        return true
      end
    end
    
    false
  end
  
  def self.new_for_path(source_album, source_path, transient = false)
    subclasses().each do |c|
      if Path.extension_eql?(c.extension, source_path)
        result = c.new
        result.source_album = source_album
        result.path = source_path
        result.transient = transient
        return result
      end
    end
    
    raise "No source track support for files of type '#{source_path.extension}' (#{source_path})."
  end
  
  def self.extension
    abstract 
  end
  
  def initialize
    symboldefaults_init(SourceFile)
  end
  
  def clean
    modificationwatcher_clean
  end
  
  def dest_path(output_format, source_album = source_album())
    raise "No source album" if !source_album
    
    source_album.dest_path(output_format).dup
  end
  
  def dirty?
    modificationwatcher_dirty?
  end
  
  def extract_raw_sound_data(source_track)
    abstract
    #[raw data file paths]
  end
  
  # Returns an array of SourceTracks defining any track data that can be retrieved from the source track.
  # Derived classes should override _extract_source_tracks instead of this method.
  # Throws ExtractTrackException
  def extract_source_tracks
    result = _extract_source_tracks
    extract_metadata(result) if !result.empty? && format_has_metadata?
    result
  end
  
  def _extract_source_tracks
    abstract
  end
  protected :_extract_source_tracks
    
  def lossy?
    abstract
  end
  
  # This method should extract any metadata from the supplied source tracks and set any applicable
  # data in the source track objects.
  def extract_metadata(source_tracks)
    abstract
  end
  
  def format_has_metadata?
    abstract
  end
  
  def length
  end
  
  def post_export(raw_data_file_paths)
  end
  
  def sound_file
    abstract
  end
  
  def title
    path().file.no_extension.literal
  end
  
  def to_s
    path().to_s
  end
  
  def viewable_name
    if path()
      path().file.literal
    else
      '(no file)'
    end
  end
end

EnumerableSubclasses.require('media/source_file_*')
