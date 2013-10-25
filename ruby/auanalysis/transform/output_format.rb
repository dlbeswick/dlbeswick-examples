require 'media/source_track'
require 'transform/transform'
require 'transform/external_program'
require 'transform/metadata'

class OutputFormat
  include Abstract
  include ConfigYAML
  include EnumerableSubclasses
  include Raise
  include SymbolDefaults
  include Viewable
  
  attr_infer :export_path_override
  attr_model_value :export_path_override, Path.new('c:/music')
  
  persistent_reader :transcoder
  persistent_reader :export_path_override
  
  symboldefaults_init

  def description
    output_extension()
  end
  
  def self.output_extension
    abstract
  end
  
  def output_extension
    self.class.output_extension
  end

  def export_base_path(library_dest_path)
    if export_path_override()
      export_path_override()
    else 
      library_dest_path
    end
  end
    
  def export_track(output_path, extracted_sound_channel_paths, source_track)
    output_path.make_dirs
      
    raise "Output path #{output_path} exists." if output_path.exists?
    
    make_output(output_path, extracted_sound_channel_paths)
    
    post_make_output(output_path, source_track)
  end
  
  def make_output(output_path, extracted_sound_channel_paths)
    abstract
  end
  
  def post_export_album(exported_files, source_album)
    verify_post_export(exported_files, source_album)
  end
  
  def post_make_output(output_path, source_track)
  end
  
  def to_s
    output_extension()
  end
  
  def valid_for?(source_track)
    abstract
  end
  
protected
  def verify_post_export(exported_files, source_album)
    if exported_files.length != source_album.tracks.length
      raise "An exported file path must be given for each source track in the source album."
    end
  end
end

class OutputFormatMetadataSupport < OutputFormat
  include Abstract
  
  persistent_reader :metadata
  persistent_reader :replaygain
  
  def post_export_album(exported_files, source_album)
    super
    
    Progress.run(nil, "Replaygain: #{self}") do |progress|
      replaygain.execute(exported_files)
    end
  end
  
  def post_make_output(output_path, source_track)
    write_metadata(output_path, source_track)
  end

  def write_metadata(output_path, source_track)
    Progress.run(nil, "Set metadata for #{output_path}") do |progress|
      metadata.set_track_info(output_path, source_track)
    end
  end
end

class OutputFormatTranscoding < OutputFormatMetadataSupport
  include Abstract

  persistent_accessor :always_transcode
  default :always_transcode, false

  def make_output(output_path, extracted_sound_channel_paths)
    if transcoder() != nil
      Progress.run(nil, "Transcoding to #{output_path}") do |progress|
        transcoder().execute(extracted_sound_channel_paths, output_path)
      end
    end
  end

  def valid_for?(source_track)
    if always_transcode == true
      true
    else
      source_track.source_file.lossy? == false && source_track.source_file.path.extension != output_extension
    end
  end
end

class OutputFormatDuplicating < OutputFormatMetadataSupport
  include Abstract

  def make_output(output_path, extracted_sound_channel_paths)
    extracted_sound_channel_paths.first.copy(output_path)
  end

  def valid_for?(source_track)
    source_track.source_file.path.extension == output_extension
  end
end

class OutputFormatMP3 < OutputFormatTranscoding
  default :metadata, MetadataExporterID3Lib.new
  default :replaygain, ReplaygainMP3Gain.new
  default :transcoder, Lame.new
  
  symboldefaults_init

  def self.output_extension
    'mp3'
  end
end

class OutputFormatMP3Duplicate < OutputFormatDuplicating
  default :metadata, MetadataExporterID3Lib.new
  default :replaygain, ReplaygainMP3Gain.new

  symboldefaults_init
  
  def self.output_extension
    'mp3'
  end
end

class OutputFormatFlac < OutputFormatTranscoding
  default :metadata, MetadataExporterFlac.new
  default :replaygain, ReplaygainMetaflac.new
  default :transcoder, Flac.new
  
  symboldefaults_init

  def self.output_extension
    'flac'
  end
end

class OutputFormatFlacDuplicate < OutputFormatDuplicating
  default :metadata, MetadataExporterFlac.new
  default :replaygain, ReplaygainMetaflac.new

  symboldefaults_init
  
  def self.output_extension
    'flac'
  end
end
