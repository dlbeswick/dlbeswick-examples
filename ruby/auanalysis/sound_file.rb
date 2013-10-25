require 'std/abstract'
require 'std/EnumerableSubclasses'
require 'std/module_logging'
require 'std/view/Viewable'

# Include this module to indicate that the class returns sound data in response to map_range
module SoundFileMappable
  include Viewable
end

module SoundFileArray
  include SoundFileMappable
  
  def domain
    0...length()
  end
  
  def range
    (-1.0..1.0)
  end
  
  def map(value)
    self[value]
  end

  def map_range(range, num_samples)
    self[range]
  end
end

class SoundFile
  include Abstract
  include EnumerableSubclasses
	include ModuleLogging
	include Raise
	include SoundFileMappable
  
  attr_reader :origin
  
  exception_context :origin
  
  def self.from_io(io, origin)
    raise 'Not currently implemented (use from_path?)'
    result = nil
    
    subclasses().each do |c|
      can_open = c.can_open_io?(io)
      io.seek(0)
      
      if can_open
        result = c.from_io(io, origin)
        break
      end
    end
    
    raise "Can't find a suitable class to process this file." if !result
    
    result
  end
  
  def self.from_path(path)
    result = nil
    
    subclasses().each do |c|
      can_open = c.can_open_path?(path)
      
      if can_open
        result = c.from_io(path.to_file('rb'), path)
        break
      end
    end
    
    raise "Can't find a suitable class to process this file." if !result
    
    result
  end
  
  def self.can_open_path?(path)
    false
  end
  
  def self.can_open_io?(io)
    false
  end
  
  def initialize(origin)
    @origin = origin
  end

  def bytes_per_element
    abstract
  end
  
  def bytes_per_sample
    bytes_per_element() * channels()
  end
  
  def channels
    abstract
  end
  
  # Random-access of samples from the sound file.
  def data_samples(samples_range, num_samples)
    abstract
  end
  
  # returns an array of data, with one array element for each channel
  def data(sec_range)
  	samples = sec_range_to_samples(sec_range)
    data_samples(samples, samples.length)
  end
  
  def domain
  	(0...length_samples())
  end
  
  # Sequential data access.
  # See 'sequential_start'.
  def each_block(block_size, &block)
    it = sequential_start()
    
    loop do
      it, result = sequential_next(it, block_size)
      break if !result
      yield result
    end
    
    yield nil
  end
  
  # Returns the length in seconds
  def length
    length_samples() / sample_rate()
  end
  
  def length_bytes
    length_samples() * bytes_per_sample()
  end
  
  def length_samples
    abstract
  end
  
  def map(sample_number)
  	data_samples(sample_number..sample_number)
  end

  def map_range(sample_range, num_samples)
  	data_samples(sample_range, num_samples)
  end
  
  def mappable_mono
    MonoMapper.new(self)
  end
  
  def moduleLogging_id
  	super + "(#{origin()})"
  end
  
  def range
  	(-1.0..1.0)
  end
  
  def sample_rate
    abstract
  end
  
  def samples_to_seconds(samples)
    samples / sample_rate()
  end
  
  def seconds_to_samples(seconds)
  	seconds * sample_rate()
  end

  class MonoMapper
    include SoundFileMappable
    
    def initialize(sound_file)
      @sound_file = sound_file
    end
    
    def domain
      @sound_file.domain
    end
    
    def range
      @sound_file.range
    end
    
    def map_range(range, num_samples)
      @sound_file.map_range(range, num_samples).first
    end
  end  
    
  class SequentialReader
    include Raise
    
    attr_reader :sound_file
    
    def initialize(sound_file)
      @sound_file = sound_file
      @iterator = 0
    end
    
    def each_with_index(buffer=2 ** 20, &block)
      while !eof?
        it = @iterator
        yield read(buffer), it
      end
    end
    
    def each_mono_with_index(buffer=2 ** 20, &block)
      while !eof?
        it = @iterator
        yield read_mono(buffer), it
      end
    end
    
    def eof?
      @iterator >= length_samples()
    end
    
    def length_bytes
      length_samples() * @sound_file.bytes_per_sample
    end
    
    def length_samples
      @sound_file.length_samples
    end
    
    def read(samples)
      return nil if eof?
      
      samples = [length_samples() - @iterator, samples].min
      
      result = @sound_file.data_samples((@iterator...@iterator + samples), samples)
      raise "Sound file reader was not at end of file, but data_samples returned no data (#{(@iterator...@iterator + samples)})" if result.first.length == 0
      
      @iterator += samples
      
      result
    end
    
    # tbd: make this a method of the returned sound data
    # sums and averages sound channels
    def read_mono(samples)
      result = read(samples)
      return nil if !result
      factor = 1.0 / result.length
      result[0].mul!(factor)
      result[1..-1].each do |ary|
        ary.mul!(factor)
        result.first.add!(ary)
      end
      result[0]
    end
    
    def seek(sample)
      @iterator = sample
    end
    
    def tell
      @iterator
    end
    
    def write_to(io, buffer=2 ** 20)
      while !eof?
        io.write(read(buffer))
      end
    end
    
    def write_mono_to(io, buffer=2 ** 20)
      while !eof?
        io.write(read_mono(buffer))
      end
    end
  end
  
  # Sequential data access.
  # The data structure of some file formats can return data faster when sequential access is stipulated.
  # Override this function when implementing such formats.
  # Returns a SequentialReader object.
  def sequential_reader
    SequentialReader.new(self)
  end
  
  def to_s
    origin().to_s
  end

  def to_yaml_properties
    ['@origin']
  end
  
protected
  def sec_range_to_samples(sec_range)
    Range.new(sec_range.first * sample_rate(), sec_range.last * sample_rate(), sec_range.exclude_end?)
  end
end
