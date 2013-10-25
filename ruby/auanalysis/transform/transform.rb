require 'transform/external_program'

class Sox < ExternalProgram
  attr_accessor :global_parameters
  attr_accessor :input_parameters
  attr_accessor :output_parameters
  attr_accessor :effect_parameters
  
  def initialize(global_parameters='', input_parameters=[''], output_parameters='', effect_parameters='')
    symboldefaults_init(Sox)
    
    @global_parameters = global_parameters
    @input_parameters = input_parameters
    @output_parameters = output_parameters
    @effect_parameters = effect_parameters
  end
  
  def parameters(input_paths, output_path)
    result = "-V #{@global_parameters}"
    
    raise "Number of input paths doesn't equal number of input parameters (supply an array of empty arrays if no parameters). (#{input_paths.inspect}, #{input_parameters.inspect})" if input_parameters.length != input_paths.length
    
    @input_parameters.zip(input_paths) do |params, path|
      if !params.empty?
        result << " #{params}"
      end
      
      result << " \"#{path.to_sys}\""
    end
    
    if !@output_parameters.empty?
      result << " #{process_parameters(@output_parameters)}"
    end
    
    result << " \"#{output_path.to_sys}\""
    
    if !@effect_parameters.empty?
      result << " #{process_parameters(@effect_parameters)}"
    end
    
    result
  end
end

class Resampler
  include Abstract
  include ConfigYAML
  include EnumerableSubclasses
  include Raise
  include SymbolDefaults
  include Viewable
  
  persistent_accessor :bits
  persistent_accessor :dither
  persistent_accessor :normalize
  persistent_infer :rate do
    'Input rate.'
  end

  default :bits, 16
  default :dither, true
  default :normalize, false

  symboldefaults_init
  
  def execute(input_paths, output_path)
    abstract
  end
  
  def input_rate
    48000
  end
  
  def resample?
    rate().kind_of?(Numeric)
  end
end

class ResamplerSox < Resampler
  def execute(input_paths, output_path)
    global = if !@dither
      '-D'
    else
      ''
    end
    
    output_channels = 2
    raise "Too many input paths, maximum is 2 (#{input_paths.length} specified)" if input_paths.length > output_channels
    
    channels_per_file = output_channels / input_paths.length 
    
    input_params = input_paths.collect do |path|
      if path.extension != "wav" && path.extension != "flac" #tbd: better detect files with format info here
        "-t raw -r #{input_rate()} -e floating-point -b 32 --channels #{channels_per_file}"
      else
        ""
      end
    end
    
    output_params = []
    output_params << "-t wav"
    output_params << ["-b ", bits]
    output_params << ["--channels ", output_channels]
    output_params << "--combine merge" if channels_per_file != output_channels
      
    if resample?
      output_params << [" -r #{rate}"]
    end
    
    effect_params = []
    if normalize()
      effect_params << ['gain', '-n']
    end 
    
    Sox.new(global, input_params, output_params, effect_params).execute(input_paths, output_path)
  end
end

class Transcoder < ExternalProgram
  include Abstract
  include EnumerableSubclasses

  persistent_accessor :resampler

  default :resampler do
    resampler = ResamplerSox.new
    resampler.dither = false
    resampler
  end
  
  symboldefaults_init 
  
  def execute(input_paths, output_path)
    # Note: only the first file is tested for resampleability.
    if resample?(input_paths.first)
      # tbd: don't resample if input and output sampling rates are equal
      # if sox is used, wav intermediate format is not necessary to convert from flac to mp3
      resampled_file = DLB::Tempfile.new("#{input_paths.first.file.literal}_#{name}_resample", nil, 'wav')
      resampled_file.close
      
      resampler.execute(input_paths, resampled_file.path)
      
      super(resampled_file.path, output_path)
      
      resampled_file.close!
    else
      super
    end
  end
  
  def set_metadata_from_track(file, track)
    abstract
  end

  def set_replaygain(album_files)
    abstract
  end

  # True if file should be resampled.
  def resample?(input_path)
    # tbd: should have a SourceFile "internal auanalysis" format instead of check for 'tmp'
    input_path.extension == "tmp" ||
      resampler.nil? == false && SourceFile.new_for_path(nil, input_path).lossy? == false
  end
end

class Lame < Transcoder
  persistent_accessor :preset
  persistent_accessor :version

  default :preset, "128"  
  default :version, "3.98.2" 
  
  default :resampler do
    resampler = ResamplerSox.new
    resampler.dither = true
    resampler.rate = 44100
    resampler.normalize = true
    resampler
  end
  
  symboldefaults_init
  
  def parameters(input_paths, output_path)
    result = []
      
    result << ["--preset ", preset]
    result << "-h"
    result << "-S"
    
    if input_paths
      raise "Multiple input paths are invalid." if input_paths.length > 1
      result << input_paths[0] 
      result << output_path
    else
      raise 'Unimplemented.'
      result << ' -'
    end
  end
  
  def search_dirs
    if PlatformOS.windows?
      super + super.collect do |path|
        path.no_file + version() + path.file
      end
    else
      super
    end
  end
  
  def set_metadata_from_track(file, track)
    abstract
  end

  def set_replaygain(album_files)
    abstract
  end
end

class Flac < Transcoder
  def parameters(input_paths, output_path)
    result = []
      
    result << ["-o", output_path]
    result << ["-S", "1s"]
      
    if input_paths
      raise "Multiple input paths are invalid." if input_paths.length > 1
      result += input_paths
    else
      raise 'Unimplemented.'
      result << [' -']
    end
    
    result
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
  
  def set_metadata_from_track(file, track)
    abstract
  end

  def set_replaygain(album_files)
    abstract
  end
end

# Deprecated
class TransformFormatExternal
  attr_accessor :external_program
  
  def initialize(external_program)
    @external_program = external_program
  end
  
  def execute(input_path, output_path)
    raise 'No external program specified.' if !external_program()
    
    Progress.run(0, external_program().to_s(input_path, output_path)) do |progress|
      external_program().execute(input_path, output_path)
    end
  end
  
  def execute_pipe(io)
    raise 'No external program specified.' if !external_program()
    
    length = if io.respond_to?(:length)
      io.length
    else
      io.seek(0, IO::SEEK_END)
      result = io.tell
      io.seek(0, IO::SEEK_SET)
      result
    end
    
    Progress.run(length, @external_program.to_s) do |progress|
      external_program().execute_pipe(io, $stdout, progress)
    end
  end
end

# note: not currently used
class IOSoundFileBitConverter
  include Raise
  
  attr_accessor :dither
  attr_accessor :noise_shape
  attr_accessor :output_bits
  
  def initialize(sound_file, buffer_size = 16384)
    @sound_file = sound_file
    @dither = false
    @noise_shape = true
    @output_bits = 32
    @iterator = @sound_file.sequential_reader
  end
  
  def length
    @sound_file.length_samples * @sound_file.bytes_per_sample
  end
  
  def read(bytes=nil)
    samples = if bytes.nil?
      samples = @sound_file.length_samples 
    else 
      samples = bytes / @sound_file.bytes_per_sample 
    end
    
    result = @iterator.read(samples)
    return nil if !result
    
    result = if @output_bits == 32
      result.to_s
    elsif @output_bits == 16
      raise 'Dither not implemented.' if @dither == true
      result.collect! { |data| (data * 32768.0).to_type(NArray::SINT).to_s }
    else
      raise 'Unsupported output bit depth.'
    end
    
    result
  end
  
  def seek(sample)
    @iterator.seek(sample)
  end
end
