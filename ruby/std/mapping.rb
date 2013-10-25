require 'std/EnumerableSubclasses'
require 'std/math'

module Mapping
	class Base
		include EnumerableSubclasses

		abstract
		
		attr_accessor :srcMin
		attr_accessor :srcMax
		attr_accessor :dstMin
		attr_accessor :dstMax
		attr_accessor :clamp
	
		def initialize(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp=false)
			@srcMin = srcMin.to_f
			@srcMax = srcMax.to_f
			@dstMin = dstMin.to_f
			@dstMax = dstMax.to_f
			@clamp = clamp
		end

		def *(value)
			if @clamp
				Math.clamp(calc(value), Math.min(@dstMin, @dstMax), Math.max(@dstMin, @dstMax)) 
			else
				calc(value)
			end
		end
		
		def calc(value)
			calc_for(value, @srcMin, @srcMax, @dstMin, @dstMax)
	  end
    
		def calc_for(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0)
			raise "Mapping #{self.class} cannot be used with dynamically-defined min and max parameters." 
	  end
	  
    def map(value)
      calc(value)
    end
    
    def map_range(range, num_samples)
      result = []
        
      for i in (0...num_samples) do
        result << calc(Mapping.linear(i, 0, num_samples, range.first, range.last))
      end

      result
    end
    
		def dstExtent
			(dstMax - dstMin).abs
		end

		def srcExtent
			(srcMax - srcMin).abs
	  end
  
    def domain
      srcMin()..srcMax()
    end
  
    def range
      dstMin()..dstMax()
    end
	end

	def Mapping.linear(value, srcMin, srcMax, dstMin, dstMax)
		if srcMin == srcMax
			dstMin
		else
			dstMin + (dstMax - dstMin) * (value - srcMin) / (srcMax - srcMin)
		end
	end

	class Linear < Base
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			Mapping.linear(value, srcMin, srcMax, dstMin, dstMax)
		end
	end
  
	def Mapping.linearN(value, srcMin, srcMax, dstMin, dstMax)
		if srcMin == srcMax
			dstMin
		else
			result = []
			
			value.zip(srcMin, srcMax, dstMin, dstMax) do |n_value, n_srcMin, n_srcMax, n_dstMin, n_dstMax|
				result << n_dstMin + (n_dstMax - n_dstMin) * (n_value - n_srcMin) / (n_srcMax - n_srcMin)
			end
			
			result
		end
	end

	class LinearN < Base
    # GraphView can show this class but it can only be used with Arrays. Selecting this class would
	  # cause a runtime error.
	  # A class method should be introduced allowing the class to tell if it supports a certain data type.
	  # GraphView should use this interface.
	  unenumerable  
	  
		def initialize(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp=false)
			@srcMin = srcMin
			@srcMax = srcMax
			@dstMin = dstMin
			@dstMax = dstMax
			@clamp = clamp
		end
		
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			Mapping.linearN(value, srcMin, srcMax, dstMin, dstMax)
		end
	end

	# shouldn't derive from Base because interface is different.
	# the 'calc' interface should be removed from Base.
	# an intermediate class should be defined for those mapping currently deriving from Base.   
  class Gain_db < Base
    unenumerable
    
    def self.calc(value, srcMin=0.0, srcMax=1.0, dbMin=-144.0)
      normalised = Mapping.linear(value, srcMin, srcMax, 0.0, 1.0)
      
      return dbMin if normalised == 0.0
      
      [20.0 * Math.log(value.abs / srcMax) / Math.log(10.0), dbMin].max
    end
    
		def calc_for(value, srcMin=0.0, srcMax=1.0, dbMin=-144.0)
			self.calc(value, srcMin, srcMax, dbMin)
		end
  end
  
  class Gain_db_Inverse < Base
    unenumerable
    
    def self.calc(value, dstMin, dstMax, dbMin=-144.0)
      return dstMin if value <= dbMin
      Mapping.linear(10 ** (value / 20.0), 0.0, 1.0, dstMin, dstMax)
    end
    
    def calc_for(value, srcMin, srcMax, dstMin, dstMax)
      self.calc(value, srcMin, srcMax, dstMin, dstMax)
    end
  end

  class Gain_dbSymmetric < Base
    unenumerable
    
    def self.calc(value, srcMag=1.0, dbMax=144.0)
      result = Mapping.linear(Gain_db.calc(value.abs, 0.0, srcMag, -dbMax), -144.0, 0.0, 0.0, 144.0)
      
      if value < 0
        -result
      else
        result
      end
    end
    
    def calc_for(value, srcMag=1.0, dbMax=144.0)
      self.calc(value, srcMag, dbMax)
    end
  end
	
  class Log < Base
  	attr_reader :base
  	
  	# The output of a linear mapping from srcMax to srcMin based on 'value' in calculated.
  	#   The output ranges from 0.0 to 1.0. 
  	# A log function is applied to the mapping, skewing the normalised linearly mapped value.  
  	# The skewed mapping is fed to a linear mapping producing output ranging from dstMin to dstMax. 
    # In summary, the formula applied is: 
    #   dstMin + (dstMax - dstMin) * (log_base(1.0 + base * (input_value - srcMin) / (srcMax - srcMin)))
  	def initialize(srcMin=0.0, srcMax=1.0, dstMin=0.0, dstMax=1.0, base=0.001)
  		super
      @base = base
  		recalc_internals()
  	end
  	
  	def recalc_internals
 			@divisor = Math.log(@base)
  		@log_domain_mapping = Mapping::Linear.new(@srcMin, @srcMax, 1.0, @base)
  	end
  	
  	def srcMin=(value)
  		@srcMin = value
  		recalc_internals
  	end
  	
  	def srcMax=(value)
  		@srcMax = value
  		recalc_internals
  	end
  	
  	def dstMin=(value)
  		@dstMin = value
  		recalc_internals
  	end
  	
  	def dstMax=(value)
  		@dstMax = value
  		recalc_internals
  	end
  	
		def calc(value)
			log_domain_input = @log_domain_mapping.calc(value)
      mapping_input = if value == 0
			  @dstMin
      else
        # This is log#{@base}(log_domain_input) (change of log base from natural log to @base)
        Math.log(log_domain_input) / @divisor
      end
      
      Mapping.linear(mapping_input, 0.0, 1.0, @dstMin, @dstMax) 
    end
  end
	
	def Mapping.power(value, power, srcMin, srcMax, dstMin, dstMax)
		srcAlpha = (value - srcMin) / (srcMax - srcMin)
		return dstMin + (dstMax - dstMin) * srcAlpha ** power
	end

	class Scalar < Base
    attr_accessor :scale
    
		def initialize(scale=1.0, srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false)
			super(srcMin, srcMax, dstMin, dstMax, clamp)
			@scale = scale
		end

		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return Mapping.linear(value * @scale, srcMin, srcMax, dstMin, dstMax)
		end
	end

	class AllToOne < Base
		attr_accessor :value
	
		def initialize(value=1.0)
			super()
			@value = value
		end

		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return @value
		end
	end

	class Gate < Base
		attr_accessor :srcCutoff

		def initialize(srcCutoff=0.5)
			super()
			@srcCutoff = srcCutoff
		end

		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			if value < self.srcCutoff
				return dstMin
			else
				return dstMax
			end
		end
	end
	
	class Power < Base
		attr_accessor :power
		
		def initialize(power=2.0, srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false)
			super(srcMin, srcMax, dstMin, dstMax, clamp)
			@power = power
		end
		
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return Mapping.power(value, @power, srcMin, srcMax, dstMin, dstMax)
		end
	end

	class Composite < Base
		attr_accessor :linkSourceRange
		attr_accessor :linkDestinationRange
		attr_reader :mappings

		abstract
		
		def initialize(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false)
			super
			@linkSourceRange = true
			@linkDestinationRange = true
			@mappings = [self.initialMapping]
			validateMappings
		end
		
		def initialMapping
			Mapping::Linear.new
		end

		def link=(b)
			@linkSourceRange = b
			@linkDestinationRange = b
		end

		def <<(mapping)
			setupMapping(mapping)
			@mappings << mapping
			self
		end
		
		def mappings=(array)
			@mappings = array
			validateMappings
		end
		
		def srcMin=(v)
			super

			return if !@linkSourceRange

			@mappings.each do |m| 
				m.srcMin=v
			end

			validateMappings
		end

		def srcMax=(v)
			super

			return if !@linkSourceRange

			@mappings.each do |m|
				m.srcMax=v
			end

			validateMappings
		end

		def dstMin=(v)
			super
			
			return if !@linkDestinationRange
			
			@mappings.each do |m|
				m.dstMin=v
			end

			validateMappings
		end

		def dstMax=(v)
			super

			return if !@linkDestinationRange

			@mappings.each do |m|
				m.dstMax=v
			end

			validateMappings
		end
		
	protected
		def setupMapping(mapping)
			mapping.srcMin=self.srcMin if @linkSourceRange
			mapping.srcMax=self.srcMax if @linkSourceRange
			mapping.dstMin=self.dstMin if @linkDestinationRange
			mapping.dstMax=self.dstMax if @linkDestinationRange
			mapping.clamp=self.clamp
		end
		
		def validateMappings
			@mappings.each do |m|
				setupMapping(m)
			end
		end
	end

	class MultiplierSequence < Composite
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			value = 1.0
			
			@mappings.each do |m|
				value = m * value
			end
			
			value
		end
	end

	class Sequence < Composite
		attr_accessor :normalizedSrc
		attr_accessor :normalizedDest
		
		def initialize(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false)
			super
			@normalizedSrc = true
			@normalizedDest = true
			@linkSourceRange = false
			@linkDestinationRange = false
		end

		def initialMapping
			Mapping::Linear.new(0.0, 1.0, 0.0, 1.0)
		end

		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return value if @mappings.empty?
			
			mapping = mappingForValue(value)
			
			outputValue(mapping.calc(mappingInputValue(value, mapping)))
		end
		
		def mappingForValue(value)
			raise "No mappings added." if @mappings.empty?
			
			value = internalValue(value)
			
			best = nil

			@mappings.each do |m|
				if m.srcMin <= value && m.srcMax >= value
					best = m
				end
			end
			
			best = @mappings.last if !best
			best
		end

	protected
		def internalValue(value)
			if @normalizedSrc
				Mapping.linear(value, @srcMin, @srcMax, 0.0, 1.0)
			else
				value
			end
		end
		
		def mappingInputValue(value, mapping)
			if @normalizedSrc
				Mapping.linear(value, @srcMin, @srcMax, 0.0, 1.0)
			else
				value
			end
		end
		
		def outputValue(value)
			if @normalizedDest
				Mapping.linear(value, 0.0, 1.0, @dstMin, @dstMax)
			else
				value
			end
		end
		
		def setupMapping(mapping)
			idx = @mappings.index(mapping)
				
			if @normalizedSrc
				mapping.srcMax = mapping.srcMax / self.srcExtent if mapping.srcMax.abs > 1.0

			  if idx > 0
					mapping.srcMin = @mappings[idx-1].srcMax
				else
					mapping.srcMin = mapping.srcMin / self.srcExtent if mapping.srcMin.abs > 1.0
				end
			end
			
			if @normalizedDest
				mapping.dstMax = mapping.dstMax / self.dstExtent if mapping.dstMax.abs > 1.0

			  if idx > 0
					mapping.dstMin = @mappings[idx-1].dstMax
				else
					mapping.dstMin = mapping.dstMin / self.dstExtent if mapping.dstMin.abs > 1.0
				end
			end
			
			mapping.clamp=self.clamp
		end
	end

	class Threshold < Composite
		attr_accessor :thresholds
		
		def initialize(srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false)
			super(srcMin, srcMax, dstMin, dstMax, clamp)
			@thresholds = [1.0]
		end
		
		def <<(mapping)
			raise "Use add instead."
		end
		
		# endValue should be normalized
		def add(endValue, mapping)
			mapping.srcMin=srcMin if @linkSourceRange
			mapping.srcMax=srcMax if @linkSourceRange
			mapping.dstMin=dstMin if @linkDestinationRange
			mapping.dstMax=dstMax if @linkDestinationRange
			@mappings << mapping
			@thresholds << endValue
		end
		
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return value if @mappings.empty?
			return mappingForValue(value).calc(value)
		end

		def mappingForValue(value)
			raise "No mappings added." if @mappings.empty?
		
			normalizedValue = Mapping.linear(value, @srcMin, @srcMax, 0.0, 1.0)

			best = nil

			(0...@thresholds.length).each do |i|
				if @thresholds[i] >= normalizedValue
					best = @mappings[i]
					break 
				end
			end
			
			best = @mappings.last if !best
			best
		end
	end

  # Accepts an array of normalized source values and their mappings to normalized destination values.
  # Interpolates between destination values using the supplied 'interpolationMapping'.
	class DataPoints < Base
		attr_accessor :dataPoints
		attr_accessor :interpolationMapping
		
		def initialize(dataPoints=[[0.0, 0.0], [1.0, 1.0]], srcMin=0.0, srcMax=0.0, dstMin=0.0, dstMax=0.0, clamp = false, interpolationMapping = Mapping::Linear.new)
			super(srcMin, srcMax, dstMin, dstMax, clamp)
			@dataPoints = dataPoints
			@interpolationMapping = interpolationMapping
		end
    
    # Makes a function from unnormalized data points. Source and dest min and max are inferred from the supplied data.
    def self.absolute(dataPoints, interpolationMapping = Mapping::Linear.new)
      raise "Empty dataPoints." if dataPoints.empty?
      
      dataPoints = dataPoints.collect { |e| e.dup }
      
      srcMin = dataPoints[0][0]
      srcMax = dataPoints[0][0]
      dstMin = dataPoints[0][1]
      dstMax = dataPoints[0][1]
      
      dataPoints[1..-1].each do |p|
        srcMin = [srcMin, p[0]].min
        srcMax = [srcMax, p[0]].max
        dstMin = [dstMin, p[1]].min
        dstMax = [dstMax, p[1]].max
      end
      
      DataPoints.new(absoluteDataPoints(dataPoints, srcMin, srcMax, dstMin, dstMax), srcMin, srcMax, dstMin, dstMax, false, interpolationMapping)
    end
    
		def calc_for(value, srcMin, srcMax, dstMin, dstMax)
			return 0.0 if @dataPoints.empty?
			
			prevVal = nil
			nextVal = nil
			
			normalizedValue = Mapping.linear(value, self.srcMin, self.srcMax, 0.0, 1.0)
			
			@dataPoints.each_index do |i|
				next if @dataPoints[i].empty?
				
				if @dataPoints[i][0] > normalizedValue
					nextVal = @dataPoints[i]
					
					if i > 0
						prevVal = @dataPoints[i-1]
					else
						prevVal = [0.0, 0.0]
					end
					
					break
				end
			end
			
			if nextVal == nil
				prevVal = @dataPoints.last
				nextVal = [1.0, prevVal[1]]
			end
			
			@interpolationMapping.srcMin = Mapping.linear(prevVal[0], 0.0, 1.0, self.srcMin, self.srcMax)
			@interpolationMapping.srcMax = Mapping.linear(nextVal[0], 0.0, 1.0, self.srcMin, self.srcMax)
			@interpolationMapping.dstMin = Mapping.linear(prevVal[1], 0.0, 1.0, self.dstMin, self.dstMax)
			@interpolationMapping.dstMax = Mapping.linear(nextVal[1], 0.0, 1.0, self.dstMin, self.dstMax)
			@interpolationMapping * value
		end
    
  protected

    # Assigns the supplied data point values and normalizes them according to the mapping's srcMin/Max and dstMin/Max settings.
    def self.absoluteDataPoints(dataPoints, srcMin, srcMax, dstMin, dstMax)
      dataPoints.each do |d|
        d[0] = Mapping.linear(d[0], srcMin, srcMax, 0.0, 1.0)
        d[1] = Mapping.linear(d[1], dstMin, dstMax, 0.0, 1.0)
      end
    end
	end
end
