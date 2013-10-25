require 'std/ConfigYAML'
require 'media/modification_watcher'
require 'std/SafeThread'
require 'std/SymbolDefaults'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'

# Source recordings for a track.
class SourceTrack
  include ConfigYAML
  include Raise
  include UniqueIDConfig
  include Viewable
  
  class ExportData
    attr_accessor :output_format
  end
  
  attr_accessor :export_data # only set during dest_path. tbd: find a better solution
  
  persistent_accessor :description
  persistent_accessor :track
  persistent_accessor :processed_sample_range
  persistent_accessor :source_album
  
  # these variables reflect metadata information from the source track.
  # both metadata and this program's implementation details dictate that there will be some
  # occasions during which it's not possible to store this data anywhere else, so it's
  # appropriate to store this data here.
  persistent_accessor :album_description
  persistent_accessor :album_release_date
  
  persistent_accessor :sample_range
  persistent_accessor :source_file
  
  persistent_infer :dest_filename_transform do
    $app.exporter.default_filename_transform
  end
    
  default :processed_sample_range, (0..0)
  default :sample_range, (0..0)
  default :album_description, ''
  default :album_release_date, ''

  default_choices :track do
    if source_album()
      source_album().tracks()
    else
      []
    end
  end
  
  default_choices :source_file do
    if source_album()
      source_album().source_files()
    else
      []
    end
  end

  def initialize(source_album, source_file, description, track = nil)
    raise "Must specify source album." if !source_album
    symboldefaults_init(SourceTrack)
    @description = description
    @source_album = source_album
    @source_file = source_file
    @track = track
  end
  
  def album
    if track()
      track().album
    else
      nil
    end
  end
  
  def dest_filename_base(output_format, dest_filename_transform=dest_filename_transform())
    subject = dup()
    
    subject.export_data = ExportData.new
    subject.export_data.output_format = output_format
    
    SaferEval.string(subject, dest_filename_transform)
  end
  
  def dest_path(output_format, source_album=source_album())
    raise "Source album required before dest_path can be evaluated." if !source_album
    # tbd: if only .localise rather than .internationalise.localise isn't done, then invalid multibyte characters can be formed
    source_album.dest_path(output_format).with_file(dest_filename_base(output_format)).internationalise.localise
  end
  
  def mark_processed
    self.processed_sample_range = sample_range()
  end

  def outdated?(output_formats)
    result = !source_file().path.exists? || processed_sample_range() != sample_range()
      
    if !result
      output_formats.each do |output_format|
        path = dest_path(output_format) 
        if !path.exists? || source_file.path().mtime > path.mtime
          result = true
          break
        end
      end
    end
    
    result
  end
  
  def outdated_status_description(output_formats)
    if !source_file.path.exists?
      return ['The source file does not exist.']
    end
    
    result = []
      
    output_formats.each do |output_format|
      path = dest_path(output_format) 
      if !path.exists?
        result << 'The destination file #{path} doesn\'t exist.'
      end
    
      if source_file.path().mtime > path.mtime
        result << 'The source file is newer than the destination file #{path}.'
      end
    end
    
    if processed_sample_range() != sample_range()
      result << 'The track boundaries have been updated.'
    end
        
    result
  end
  
  def source_album
    if @source_album
      @source_album
    elsif source_file()
      source_file().source_album
    else
      nil
    end
  end
  
  # modifies any internal data such that it is up to date with the data at the dest path. 
  def sync_with_dest
    @processed_sample_range = @sample_range
  end
  
  def to_s
    description().dup
  end
end
