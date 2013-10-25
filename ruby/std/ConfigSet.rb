require 'std/ConfigYAML'

# Allows different sets of config information to be saved and restored to a specified object.
# Currently only supports loading and saving to an in-memory string.
class ConfigSet
	# @target the object being saved and restored
	#	@configClass the Config module to beused
	def initialize(target, configClass = ConfigYAML, &block)
		@target = target
		
		# include ConfigYAML in the target if we haven't already
    if !@target.class.include?(configClass)
      raise "Target must include class '#{configClass}'"
    end
		
		# create class to include desired config class
    # tbd: classes may leak memory?
    newAnonClass = Class.new
    newClassName = "#{target.class.name}_#{target.object_id}"
    ConfigSet.const_set(newClassName.intern, newAnonClass)   
		@configObj = newAnonClass.new
		@configObj.class.module_eval("include #{configClass}")
		
		class << @configObj
			def configClassName
				@target.class.name
			end
			
      def configTarget
        @target
      end
        
			def target=(t)
				@target = t
			end
		end
		
		@configObj.target = @target
		
		yield self if block
  end

	def to_yaml_type
		"!david,2007/configset"
	end
	
	def persistent(symbol)
		@configObj.class.persistent(symbol, @target)
	end

	def persistent_accessor(symbol)
		@configObj.class.persistent_accessor(symbol, @target)
	end
	
	def load
		return if !@data
		@configObj.class.load(@target, @data, false, @configObj.configVariables)
		@data = nil
	end
	
	def save
		@data = ""
		
		StringIO.open(@data,"w") do |stream|
			@configObj.save_stream(stream)
		end
		
		puts @data
	end
end
