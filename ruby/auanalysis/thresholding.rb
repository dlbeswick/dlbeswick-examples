require 'wavelet'
require 'std/abstract'
require 'std/ObjectHost'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'

class ThresholdFunction
  include Abstract
  include EnumerableSubclasses
  include SymbolDefaults
  include UniqueIDConfig
  include Viewable

  def sure_threshold_scale
    abstract
  end

  def threshold_vector(vector, threshold)
    abstract
  end
end

class ThresholdHard < ThresholdFunction
  def sure_threshold_scale
    1.0
  end
  
  def threshold_vector(vector, threshold)
    vector_abs = NArrayMemoryMappedTemp.new(vector)
    vector_abs.abs!
    mask = vector_abs.gt(threshold)
    vector.mul!(mask)
    #result.collect! do |e|
    #  if e.abs <= threshold
    #    0.0
    #  else
    #    e
    #  end
    #end
  end
end

class ThresholdSoft < ThresholdFunction
  def sure_threshold_scale
    2.0
  end
  
  def threshold_vector(vector, vector_threshold)
    vector_threshold /= 2.0
    
    vector.collect! do |e|
      if e >= vector_threshold
        e - vector_threshold
      elsif e <= -vector_threshold
        e + vector_threshold
      elsif e.abs <= vector_threshold
        0.0
      end
    end
  end
end

class DenoiseThresholdOperation
  include EnumerableSubclasses
  include SymbolDefaults
  include UniqueIDConfig
  include Viewable
  
  symboldefaults_init
  
  def estimate_noise_variance(vector)
    vector_abs = NArrayMemoryMappedTemp.new(vector)
    vector_abs.abs!
    vector_abs.sort!
    median = vector_abs[vector_abs.length / 2]
    vector_abs.close
    #median = approximate_median(result_abs) 
    #p median
    
    median / 0.6745 # sparse way 11.85
  end
  
  def estimate_max_noise_amplitude_from_variance(noise_variance, vector_length)
    noise_variance * Math.sqrt(2.0 * Math.log(vector_length)) # sparse way pg 556
  end
  
  def estimate_threshold(vector, signal_length, noise_variance, sensitivity = 1)
    threshold = estimate_max_noise_amplitude_from_variance(noise_variance, signal_length)
    threshold * sensitivity 
  end

  def run(result, threshold=nil, sensitivity=1)
  end
end

class Bandpass < DenoiseThresholdOperation
  default_accessor :keep_range_first, 4
  default_accessor :keep_range_last, 4
  
  symboldefaults_init
  
  def run(result, threshold=nil, sensitivity=nil)
    i = 0
    keep_range = (keep_range_first..keep_range_last)
    result.each_vector do |vector|
      vector.fill!(0) if !keep_range.include?(i)
      i += 1
    end
  end
end

class DenoiseSimple < DenoiseThresholdOperation
  include ObjectHost
  
  default_accessor :threshold_function, ThresholdHard.new
  default_accessor :coloured, false
  
  host :threshold_function
  
  symboldefaults_init
  
  def run(result, threshold=nil, sensitivity=1, coloured=coloured())
    if !coloured
      finest_scale_vector = result.refer_vector(0)
      threshold = estimate_threshold(finest_scale_vector, result.length, estimate_noise_variance(finest_scale_vector), sensitivity) if !threshold
      threshold_function.threshold_vector(result, threshold)
    else
      result.each_vector do |vector|
        vector_threshold = estimate_threshold(vector, result.length, estimate_noise_variance(finest_scale_vector), sensitivity) if !threshold
        threshold_function.threshold_vector(vector, vector_threshold)
      end
    end
  end
end

class DenoiseMultiscaleSure < DenoiseThresholdOperation
  include ObjectHost
  
  unenumerable
  
  default_accessor :threshold_function, ThresholdSoft.new

  host :threshold_function
  
  symboldefaults_init
  
  def sure_vector_threshold(vector, noise_variance, small_energy_threshold, estimated_vector_threshold)
    signal_norm_estimate = vector.inner_product - vector.length * noise_variance ** 2
    
    if signal_norm_estimate <= small_energy_threshold
      noise_variance * Math.sqrt(2.0 * Math.log(vector.length))
    else
      estimated_vector_threshold
    end
  end
  
  def run(result, threshold=nil, sensitivity=1)
    finest_scale_vector = result.refer_vector(0)
    
    noise_variance = estimate_noise_variance(finest_scale_vector)
    noise_threshold_estimate = estimate_threshold(finest_scale_vector, result.length, noise_variance, sensitivity)
    
    result.each_vector do |vector|
      small_energy_threshold = (noise_variance ** 2) * Math.sqrt(vector.length) * (Math.log(vector.length) ** (3.0/2.0))
           
      vector_threshold = sure_vector_threshold(vector, noise_variance, small_energy_threshold, noise_threshold_estimate)
      
      vector_threshold *= threshold_function.sure_threshold_scale 
      
      threshold_function.threshold_vector(vector, vector_threshold)
    end
  end
end

class ThresholdWaveletDiscreteHaar < WaveletDiscreteHaar
  include ObjectHost
  include SymbolDefaults
  
  default_accessor :auto_threshold_sensitivity, 1
  attr_infer :null_cutoff_vector_idx do 10; end
  attr_infer :null_all_but_idx do -1; end
  attr_infer :threshold do 'auto'; end
  default_accessor :denoise_operation, DenoiseMultiscaleSure.new
  
  host :denoise_operation
  
  def initialize(source=nil)
    super(source)
    symboldefaults_init(ThresholdWaveletDiscreteHaar)
  end
  
  def null_nominated_vectors(result)
    if null_all_but_idx() != -1  
      i = 0
      result.each_vector do |vector|
        vector.fill!(0) if i != null_all_but_idx()
        i += 1 
      end
    end
  end

  def calc(sound_data, result=nil)
    result = super
    
    null_nominated_vectors(result)
    
    denoise_operation().run(result, nil, auto_threshold_sensitivity())
    
    result
  end
end
