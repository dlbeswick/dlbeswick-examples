require 'std/platform'
require 'data_source'
require 'std/view/View'
require 'std/view/GraphView'
require 'std/progress'
require 'wavelet'
require 'narray'
#require 'numru/fftw3'
#module FFTW
#  def self.fftw(*args)
#    result = NumRu::FFTW3.fft(*args)
#    p result
#    result
#  end
#end

class ViewSoundFile < GraphView
	include EnumerableInstances
	
	abstract
	
	suits(:SoundFileMappable)
	
	def initialize(target, parentWindow, renderer)
		super
		renderer.gridDivisions = 0

		self.view_domain_start = 0
    self.view_domain_size = 128
    self.range = -1.0..1.0
	end
	
	def make_controls
		super
		
		layoutAddVertical(
			makeCheck(self, "Master") do |state|
				if state
					self.class.give_master_to(self)
				else
					self.class.give_master_to(nil)
				end
			end
		)

    layoutAddVertical(
      makeCheck(self, "dB") do |state|
        
        if !renderer().respond_to?(:display_db=)
          class << renderer()
            def display_db=(b)
              @display_db = b
            end
            
            def postprocess_sampled_data(data, desired_samples, drawing_area_width)
              data = super
              
              if @display_db
                data.collect! do |array|
                  array.collect! do |e|
                    if e.respond_to?(:[])
                      [Mapping::Gain_dbSymmetric.calc(e[0]), Mapping::Gain_dbSymmetric.calc(e[1])]
                    else
                      Mapping::Gain_dbSymmetric.calc(e)
                    end
                  end
                end
              end
              
              data
            end
            
            def range
              if @display_db
                (-144.0..144.0)
              else
                super
              end
            end
          end
        end
        
        renderer().display_db = state
        
        redraw
      end
    )
	end
	
	def self.give_master_to(new_master)
		ViewSoundFile.instances.each do |i|
			i._is_master = i.equal?(new_master)
		end
			
		if new_master
			new_master.other_instances().each do |i|
				i.view_domain_start = new_master.view_domain_start()
  		end
  		
			new_master.other_instances().each do |i|
				i.view_domain_size = new_master.view_domain_size()
  		end
		end
	end
	
	def _is_master=(b)
		@is_master = b
	end
	
protected
	attr_reader :is_master

	def slaves
	  other_instances()
	end
	
  def updateViewDomain
  	super
  	
  	if is_master()
			slaves().each do |i|
				i.view_domain_start = view_domain_start()
  		end
  	end
  end

  def updateScrollDomain
  	super
  	
  	if is_master()
			slaves().each do |i|
				i.view_domain_size = view_domain_size()
  		end
  	end
  end
end

class Threshold_dB
  include DataSource2D
  
  attr_accessor :threshold
  
  def initialize(source, threshold=-144.0)
    super(source)
    @threshold = threshold
  end
  
  def map_range(sample_range, num_samples)
    thresh = threshold() + 144.0

    @source.map_range(sample_range, num_samples).collect do |v|
      db = Mapping::Gain_dbSymmetrical.calc(v)
      
      if db.abs < thresh
        0.0
      else
        db / 144.0
      end
    end
  end
end

class ViewSoundFileAmplitude < ViewSoundFile
  default_reader :scale_db, false
  
	def initialize(target, parent, renderer=GraphRendererContinuousMinMax.new(target.get))
    super
    
    class << renderer()
      attr_accessor :scale_db
      
      def process_data(data, drawing_area_width)
        if scale_db() == true
          data.collect! do |v|
            Mapping::Gain_db.calc(v.abs, 0.0, 1.0)
          end
        end
        
        super
      end 
    end
    
    symboldefaults_init(ViewSoundFileAmplitude)
  end
  
  def make_controls
    super
    
    layoutAddVertical(
      makeCheck(self, "dB scale") do |state|
        self.scale_db = state
        redraw()
      end
    )
  end
  
  def scale_db=(b)
    renderer().scale_db=b
    
    if b
      renderer().range = (-144.0..0.0)
    else
      renderer().range = (-1.0..1.0)
    end
  end
  
  def sound_file
  	target.get
  end
  
	def self.suits_target?(target, target_class)
    target_class <= SoundFileMappable || target_class <= DataSource 
	end
end

def circular_slice(array, range)
	raise "Slice too large" if range.length > array.length
	range = (range.first..range.last % array.length)
	
	if range.last < range.first
		array[range.first..-1] + array[0..range.end]
	else
		array[range]
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

def maxima(input, threshold)
	result = [0]
	
	1.upto(input.length - 2) do |i|
		a = input[i - 1]
		b = input[i]
		c = input[i + 1]
		
		ba = b - a
		ca = c - a
		
		result << if (ba >= threshold && ca < threshold) || (ba < threshold && ca >= threshold) \
		  || (ca >= threshold && ba < threshold) || (ca < threshold && ba >= threshold)
			b
		else
			0
		end
		#result << Mapping.linear([ba,ca].max,0.0,96.0,0.0,1.0)
	end
	
	result << 0
	
	result
end

class Maxima2D
	include Viewable
	
	attr_accessor :threshold
	
	def initialize(source, threshold=-48.0)
		@source = source
		@threshold = threshold
	end
	
  def map_range(sample_range, num_samples)
  	@source.map_range(sample_range, sample_range.length).collect do |band|
  	  maxima(band, 0).collect do |x|
  	    db = Mapping::Gain_db.calc(x, 0.0, 1.0)

  	    if db < threshold()
  	      -144.0
  	    else
					db
  	    end
  	  end 
  	end
  end

	def domain
		@source.domain
	end

	def range
		@source.range
	end
end

class Threshold_dB_2D
  include DataSource2D
  include Viewable
  
  attr_accessor :threshold
  
  def initialize(source, threshold=0.0)
    super(source)
    @threshold = threshold
  end
  
  def map_range(sample_range, num_samples)
    thresh = threshold() + 144.0
    
    @source.map_range(sample_range, num_samples).collect do |band|
      band.collect! do |v|
        db = Mapping::Gain_dbSymmetrical.calc(v)
        
        if db.abs < thresh
          0.0
        else
          db / 144.0
        end
      end
    end
  end
  
  def range
    (-144.0..144.0)
  end
end

class Autocorrelation
  include DataSource2D

  def initialize(source, threshold=0.0)
    super(source)
    @threshold = threshold
  end

  def map_range(sample_range, num_samples)
    @source.map_range(sample_range, sample_range.length).collect do |band|
      result = []
      
      band_convolve = band * 2
       
      n = band.length
      n_r = 1.0 / n
      max = 0
      
      ProgressStdout.new(n) do |progress|
        n.times do |i|
          element = 0
          
          0.upto(n - 1) do |j|
            element += band_convolve[j] * band_convolve[j + i]
          end
          
          #element *= n_r
          
          result << element
          
          max = element if element.abs > max 
          
          progress << 1
        end
      end
      
#      result.collect! { |x| x / max }
      
      p result  
      result
    end
  end
end

class ViewSoundFileFrequencyWavelet < ViewSoundFile
	include Viewable
	
	suits(Wavelet)
	
	attr_accessor :threshold
	
	def initialize(target, parentWindow)
    renderer = GraphRendererIntensity2D.new(target.get)

    super(target, parentWindow, renderer)
  end
end

class ViewSoundFileWaveletAmplitude < ViewSoundFile
  include Viewable
  
  suits(DataSource2D)
  
  def initialize(target, parent)
    super(target, parent, GraphRendererContinuousMinMax2D.new(target.get, (1..9999)))
  end

  def make_controls
    super
    
    @text_min_idx = layoutAddHorizontal(
      makeText(self, renderer().range_view_slice().first.to_s) do |text|
        renderer().range_view_slice = (text.to_i..renderer().range_view_slice.last)
        repaint()
      end
    )

    @text_max_idx = layoutAddHorizontal(
      makeText(self, renderer().range_view_slice().last.to_s) do |text|
        renderer().range_view_slice = (renderer().range_view_slice.first..text.to_i)
        repaint()
      end
    )
  end
end

class ViewSoundFileWaveletAmplitude_dB < ViewSoundFileWaveletAmplitude
  def initialize(target, parent)
    super
    
    self.range = -144.0..144.0
    
    class << renderer().slice_renderer()
      def process_data(data, drawing_area_width)
        data.collect! do |v|
          Mapping::Gain_dbSymmetrical.calc(v)
        end
        
        super
      end 
    end
  end
end

class ViewSoundFileWaveletAmplitude_dB_Abs < ViewSoundFileWaveletAmplitude
  def initialize(target, parent)
    super
    
    self.range = -144.0..0.0
    
    class << renderer().slice_renderer()
      def postprocess_sampled_data(data, desired_samples, drawing_area_width)
        minmax = [144,-144]
        
        data.collect! do |v|
          result = Mapping::Gain_db.calc(v.abs, 0.0, 1.0)
          minmax[0] = [minmax[0], result].min
          minmax[1] = [minmax[0], result].max
          result 
        end
        
        super
      end 
    end
  end
end

class ViewSoundFileMaxima < ViewSoundFileFrequencyWavelet
  suits(Maxima2D)
  
	def initialize(target, parent)
		super
    self.range = -144.0..0.0
	end
	
	def make_controls
		super
		layoutAddVertical(View.newViewFor(target.get, nil, self, Views::AllAttributes.subclasses))
	end
end

if false
class ViewSoundFileFrequencyFFT < ViewSoundFile
	include NumRu
	
	suits(SoundFileMappable)
	
	attr :window_size
	
	def initialize(target, parentWindow)
		window_size = 64
		
		sound_file = target.get
		
    renderer = GraphRendererIntensityN.new(sound_file) do |renderer, view_domain, num_samples|
      i = 0.0
      
      data_samples = [num_samples + window_size, sound_file.length_samples - view_domain.first].min
      data = target.get.map_range(view_domain, data_samples)
      padding = [0.0, window_size - data.length].max
      data = NArray.to_na(data + [0.0] * padding)
      
      result = []
      
      #mapping = Mapping::Linear.new(-window_size * 0.25, window_size * 0.25, 0.0, 1.0)
      
      num_samples.times do |i|
      	#p data[i...i + window_size].max
        fft = FFTW3.fft(data[i...i + window_size], -1)
        #fft /= fft.length
        
	      magnitudes = [] 
	      fft[0...fft.length/2].each do |e|
	      	mag = e.real * e.real + e.imag * e.imag
	      	mag = 2.0 * mag / fft.length
	      	magnitudes << mag
	      end
	      
	      p [magnitudes.min, magnitudes.max]
        result << magnitudes
      end

      result
    end

    super(target, parentWindow, renderer)

    window_width = 11025
    
    self.range = 0.0..1.0
    self.view_domain_start = sound_file.seconds_to_samples(30.0)
    self.view_domain_size = window_width
  end
end
end
