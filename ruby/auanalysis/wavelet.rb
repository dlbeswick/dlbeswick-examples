PlatformOS.require('std/dsound')
require 'std/file_memory_mapped'
require 'data_source'
require 'narray_auanalysis'

module WaveletResult
  include DataSource2D
  
  def collect_vectors(&block)
    result = []
      
    (0...length_vectors()).each do |i|
      result << yield(refer_vector(i))
    end
    
    result
  end
  
  def map_range(range, desired_samples)
    result = []
    range = range.normalise_inclusive
      
    source_length = domain().length
    
    each_vector() do |vector|
      ratio = vector.length / source_length.to_f 
      result << vector[(range.first * ratio).to_i..(range.last * ratio).to_i]
    end
    
    result
  end
  
  def domain
    (0...length())
  end
  
  def range
    (-1.0...1.0)
  end
  
  def each_vector(&block)
    (0...length_vectors()).each do |i|
      yield refer_vector(i)
    end
  end
  
  def length_vectors
    (Math.log(length()) / Math.log(2)).floor
  end
  
  def refer_vector(vector_idx)
    begin
      refer_slice((length() - length() / (2**vector_idx))...(length() - length() / (2**(vector_idx+1))))
    rescue
      $stderr.puts '---'
      $stderr.puts 'Error during "refer_vector", check memory status.'
      $stderr.p vector_idx
      $stderr.p length()
      $stderr.p length_vectors()
      $stderr.p((length() - length() / (2**vector_idx))...(length() - length() / (2**(vector_idx+1))))
      raise
    end
  end
  
  def split
    result = []
    
    i = 0
    input_size = length() / 2
    
    while input_size > 0
      result << refer_slice(i...i+input_size)
      i += input_size
      input_size /= 2
    end
    
    result
  end
end

module WaveletResultMemoryMapped
  def self.sfloat(path_or_tempfile)
    xx 'aa'
    path = if path_or_tempfile.kind_of?(DLB::Tempfile)
      xx 'bb'
      @keep_tempfile = path_or_tempfile
      @keep_tempfile.path
    else
      xx 'cc'
      path_or_tempfile
    end 
      
    xx 'dd'
    result = NArrayMemoryMapped.sfloat(path)
    xx 'ee'
    result.extend(WaveletResult)
    xx 'ff'
    result
  end
end

if false
class WaveletDiscreteHaar_ruby < WaveletDiscrete
  def initialize(source, wavelet, scaling)
    @wavelet = [1.0 / Math.sqrt(2)] * 2 
    @scale = [1.0 / Math.sqrt(2), -1.0 / Math.sqrt(2)] 

    super(source) 
  end
protected
  def step(input, reverse_wavelet, reverse_scaling)
    result = [[], []]
    
    length = input.length
    num_coefficients = reverse_wavelet.length
    input = input + input[0...num_coefficients]

    if false
      (0...length).step(2) do |i|
        result[0] << FFTW.convol(NArray.to_na(reverse_scaling), NArray.to_na(input[i, num_coefficients]))
        result[1] << FFTW.convol(NArray.to_na(reverse_wavelet), NArray.to_na(input[i, num_coefficients]))
      end
    else
    
    #ProgressStdout.new(length, "Wavelet transform #{input.length} samples.") do |progress|
      (0...length).step(2) do |i|
        rs = 0
        rw = 0
        
        0.upto(num_coefficients - 1) do |k|
          rs += input[i + k] * reverse_scaling[k]
          rw += input[i + k] * reverse_wavelet[k]
        end
        
        result[0] << rs
        result[1] << rw
        
        #progress << 2
      end
    #end
    end
    
    result
  end
  
  def calc(input, wavelet, scaling, max_steps=@steps)
    return input if input.length == 0
    
    result = []
    
    wavelet_reverse = @wavelet.reverse
    scaling_reverse = @scaling.reverse
    
    iterations = 0
    
    while iterations < max_steps && input.length > 2
      step_result = step(input, wavelet_reverse, scaling_reverse)
      input = step_result[0]
      result = result + [step_result[1]]
      iterations += 1
    end
    
    result
  end
end
end

module WaveletWithSoundFile
  def mix_data!(sound_data)
    result = sound_data.first
    factor = 1.0 / sound_data.length
    sound_data[1..-1].each do |ary|
      result += ary * factor
    end
    result
  end

  def data_from_source(sample_range, num_samples)
    mix_data!(super)
  end
end

class Wavelet
  include DataSource2D
end

class WaveletDiscrete < Wavelet
protected
  def calc(data)
  end
end

class WaveletDiscreteHaar < WaveletDiscrete
  attr_reader :native
  
  no_view :native
  
  def initialize
    super
    @native = Dsound::WaveletLiftingHaar.new
  end

  def calc_disk_tmpfile(data_path)
    result = DLB::Tempfile.ready(self.class.name)
    calc_disk(data_path, result.path)
    result
  end
  
  def calc_disk(data_path, result_path, work_path=nil)
    if !work_path
      tmp_work = DLB::Tempfile.ready('wavelethaar_work')
      work_path = tmp_work.path
    end
    
    @native.calc(data_path.to_sys, result_path.to_sys, work_path.to_sys)
    
    result_path
  end
  
  def calc(data, result=nil)
    if !data.kind_of?(NArray)
      raise "Parameter must be NArray (#{data.class} given.)" 
    end
    
    if data.dim != 1
      raise "NArray must have single dimension." 
    end

    result = allocate_result(@native.output_size_for(data.length)) if result == nil
     
    work = allocate_result(result.length)
    @native.calc(data.to_buffer, data.length, result.to_buffer, result.length, work.to_buffer, work.length)

    deallocate_result(work)
    
    result.extend(WaveletResult)
  end
  
  def invert(wavelet_result, reconstructed = nil)
    reconstructed = allocate_result(wavelet_result.length) if reconstructed == nil
    work = allocate_result(wavelet_result.length)
    @native.invert(wavelet_result.to_buffer, wavelet_result.length, reconstructed.to_buffer, reconstructed.length, work.to_buffer, work.length)
    deallocate_result(work)
    reconstructed
  end

  def invert_disk(data_path, result_path, work_path=nil)
    if !work_path
      tmp_work = DLB::Tempfile.ready('wavelethaar_invert_work')
      work_path = tmp_work.path
    end
    
    @native.invert(data_path.to_sys, result_path.to_sys, work_path.to_sys)
    
    result_path
  end
  
  def map_range(sample_range, num_samples)
    calc(data_from_source(sample_range, num_samples)).split
  end
  
protected
  def allocate_result(input_size_elements)
    NArray.sfloat(input_size_elements)
  end

  def deallocate_result(result)
    result.close if result.respond_to?(:close)
  end
end
