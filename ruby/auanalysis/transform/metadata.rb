require 'std/tempfile_dlb'
require 'transform/external_program'
require 'id3lib'

class MP3Gain < ExternalProgram
  def parameters(input_paths, output_path)
    if PlatformOS.windows?
      ['/p', '/s r', '/q'] + input_paths
    else
      ['-p', '-s r', '-s i'] + input_paths
    end
  end
end

class Metaflac < ExternalProgram
  attr_accessor :apply_replaygain
  attr_accessor :output_metadata
  
  def initialize
    super
  end
  
  def execute(input_paths, output_path)
    begin
      @metadata_file.close if @metadata_file
      super
    ensure
      @metadata_file.close! if @metadata_file
      @metadata_hash = nil
    end
  end
  
  def metadata=(hash)
    @metadata_file = DLB::Tempfile.new
    @metadata_file.keep!
    @metadata_hash = hash

    hash.each do |k, v|
      @metadata_file.puts("#{k}=#{v}")
    end
  end
  
  def parameters(input_paths, output_path)
    result = []
      
    if @metadata_file
      if !@metadata_hash.empty?
        result << ["--remove-all-tags"]
      end
      
      result << ["--import-tags-from=", @metadata_file.path]
    end
    
    if @apply_replaygain
      result << "--add-replay-gain"
    end
    
    if @output_metadata
      result << "--export-tags-to=-"
    end
    
    result += Array(input_paths) 
    
    result
  end

  def search_path_dir
    if PlatformOS.windows?
      'flac'
    else
      super
    end
  end
  
  def search_dirs
    if PlatformOS.windows?
      super + super.collect do |path|
        path + 'bin'
      end
    else
      super
    end
  end
end

class MetadataExporter
  include Abstract
  include ConfigYAML
  include EnumerableSubclasses
  include Raise
  include SymbolDefaults
  include Viewable
  
  def set_track_info(file, source_track)
    abstract
  end
end

class MetadataExporterFlac < MetadataExporter
  include EnumerableSubclasses
  
  def set_track_info(file, source_track)
    metaflac = Metaflac.new
    metadata = {}
    metadata['ARTIST'] = source_track.track.artist
    metadata['ALBUM'] = source_track.album.title 
    metadata['TITLE'] = source_track.track.title
    metadata['TRACKNUMBER'] = source_track.track.order
    metadata['DATE'] = source_track.track.release_date
    metaflac.metadata = metadata
    metaflac.execute(file, nil)
  end
end

class MetadataExporterID3 < MetadataExporter
  include EnumerableSubclasses
end

class MetadataExporterID3Lib < MetadataExporterID3
  def set_track_info(file, source_track)
    tag = ID3Lib::Tag.new(file.to_sys)
    tag.strip!
    tag.artist = source_track.track.artist
    tag.album = source_track.album.title 
    tag.title = source_track.track.title
    tag.track = source_track.track.order
    tag.year = source_track.track.release_year
    tag.update!
  end
end

class Replaygain
  include Abstract
  include ConfigYAML
  include Raise
  include SymbolDefaults
  include Viewable
  
  def execute(input_files)
    abstract
  end
end

class ReplaygainMetaflac < Replaygain
  include EnumerableSubclasses
  
  def execute(input_files)
    metaflac = Metaflac.new
    metaflac.apply_replaygain = true
    metaflac.execute(input_files, nil)
  end
end

class ReplaygainMP3Gain < Replaygain
  def execute(input_files)
    MP3Gain.new.execute(input_files, nil)
  end
end
