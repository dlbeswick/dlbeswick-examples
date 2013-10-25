require 'sound_file'
require 'std/file_memory_mapped'
require 'narray'
require 'std/raise'

class SoundFileRaw < SoundFile
  include Raise
  
  attr_accessor :bytes_per_element
  attr_accessor :channels
  attr_accessor :sample_rate
  
  def self.from_path(path, channels, sample_rate, bytes_per_element)
    raise "Not implemented" if bytes_per_element != 4
    self.new(path, NArrayMemoryMapped.sfloat(path), channels, sample_rate, bytes_per_element)
  end
  
  def initialize(origin, narray, channels, sample_rate, bytes_per_element)
    @channels = channels
    @sample_rate = sample_rate
    @bytes_per_element = bytes_per_element
    @narray = narray
    super(origin)
  end

  def data_samples(samples_range, num_samples)
    if samples_range.first < 0 || samples_range.last > length_samples()
      raise "Supplied range of #{samples_range} is out of the sound file's domain of #{0...length_samples()}"
    end

    @narray[samples_range]
  end
  
  def length_samples
    @narray.length / channels()
  end
end
