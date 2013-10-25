require 'std/algorithm'
require 'std/IncludableClassMethods'
require 'logger'

$modulelogging_global_log_enable = true

module ModuleLogging
  include IncludableClassMethods

  def self.global_log_enable=(b)
    $modulelogging_global_log_enable = b
  end
  
  # code generator for logging methods.
  # ruby seems to be able to optimise when 'yield' is used rather that defining a block parameter.
  # it makes the code a little more complex, so this generator aims to ease usage of the 'yield' method.  
  def self.write_log_method(where, symbol_modulelogger_responder, symbol_method, symbol_io_obj, log_level, caller_depth)
    where.module_eval <<-EOS
    def #{symbol_method}(string=nil)
      logger = #{symbol_modulelogger_responder}.moduleLogger
      raise "No logging instance found for class " + #{symbol_modulelogger_responder}.to_s if !logger
      return if !logger.enabled?(#{log_level})
      string ||= yield 
      logger.sendToOutput(#{symbol_io_obj}, self, #{caller_depth}, string)
    end
    EOS
  end
  
	class Logger
	  attr_accessor :log_time
	  
		def initialize(tag)
			@tag = tag
			@log_level = 2
		end

    def log_level
      @log_level
    end
    
		def log_level=(i)
			@log_level = i
		end
		
		def enabled=(b)
			if b
				@log_level = 3
			else
				@log_level = 1
			end
		end
		
		def enabled?(log_level = 1)
      $modulelogging_global_log_enable == true && @log_level >= log_level
		end
		
    def sendToOutput(io, instance, depth, text=nil, &block)
      callingFunction = begin
        # match everything within `', also, in 1.9 it often states "block in xxx". The 'xxx' is what
        # should be captured, so capture and ignore words and spaces before the final word.
        /`(?:(?:\w+? )*)([^']*)'/.match(caller[depth])[1]
      rescue NoMethodError
        "(Unknown method)"
      end
      
      if text      
      	logging_id = instance.moduleLogging_id
      	
      	drb_origin_id = if Object.const_defined?(:DRb)
      	  begin
      	    "@#{DRb.current_server.uri}"
      	  rescue DRb::DRbServerNotFound
      	    ''
      	  end
      	else
      	  ''
      	end 
      	
        if logging_id != nil
          header = "[#{instance.class.name}#{drb_origin_id}] #{logging_id}."
        else
          header = "[#{@tag}#{drb_origin_id}] "
        end
      
        header += callingFunction
      
        if log_time()
          header += " #{Time.now.to_f}"
        end
            
        io.puts "#{header}: #{text}"
      end
    end
  end
	
	module ClassMethods
	  def log?
      moduleLogger().enabled?(1)
	  end
	  
		def log=(enabled_or_level)
		  if enabled_or_level.kind_of?(Numeric)
		    moduleLogger().log_level = enabled_or_level
		  else 
		    moduleLogger().enabled = enabled_or_level
		  end
		  
		  string = if moduleLogger().log_level == 0
		    "Logging disabled."
		  else
        "Logging level set to #{moduleLogger().log_level}."
		  end
		    
      moduleLogger().sendToOutput($stdout, self, 1, string)
	  end
	  
    def log_time=(b)
      moduleLogger().log_time = b
    end
  
    def moduleLogger
      if !@moduleLogger
        Algorithm.each_ancestor(self, true) do |a|
          logger = a.instance_variable_get(:@moduleLogger) 
          if logger
            @moduleLogger = logger
            break
          end
        end
        
        raise "No @moduleLogger found in ancestory hierarchy for #{self}." if !@moduleLogger
      end
      
      @moduleLogger
    end
		
    # Logging class methods for ModuleLogging class clients.
    ModuleLogging.write_log_method(self, "self", :log, :$stdout, 2, 1)
    ModuleLogging.write_log_method(self, "self", :err, :$stderr, 1, 1)
    ModuleLogging.write_log_method(self, "self", :note, :$stdout, 3, 1)
    ModuleLogging.write_log_method(self, "self", :warn, :$stderr, 2, 1)
    
	  def moduleLogging_id
	  	nil
	 	end
  end

  def self.included(mod)
    IncludableClassMethods.give_classmethods(self, mod)
    # note: mod.name is sometimes not set at this point of execution, so inspect is used instead.
    mod.instance_variable_set(:@moduleLogger, ModuleLogging::Logger.new(mod.inspect))
  end

	# Logging methods for instances of ModuleLogging clients.
	# Either a block or string can be used.
	# A block may be more efficient as the contents need not be evaluated if the log level does not require it.
  # Logging methods are log, err, note, warn.
  ModuleLogging.write_log_method(self, "self.class", :log, :$stdout, 2, 1)
  ModuleLogging.write_log_method(self, "self.class", :err, :$stderr, 1, 1)
  ModuleLogging.write_log_method(self, "self.class", :note, :$stdout, 3, 1)
  ModuleLogging.write_log_method(self, "self.class", :warn, :$stderr, 2, 1)
  
  def moduleLogging_id
  	self.object_id.to_s
 	end
end
