require 'std/IncludableClassMethods'
require 'std/module_logging'
require 'std/raise'

# Ensure that class.deprecated_since is called after warning_period and fatal_period (whose use is optional).
# tbd: allow deprecation of individual methods.
module Deprecatable
  include IncludableClassMethods
  include ModuleLogging
  include Raise
  
  module ClassMethods
    def module_deprecated_since(year, month)
      elapsed = Time.now - Time.local(year, month)

      if fatal_deprecation?(elapsed)      
        raise "Module '#{self.class}' is past its fatal deprecation period -- remove references to it immediately."
      elsif warn_deprecation?(elapsed)
        ModuleLogging.do_log(Deprecatable, Deprecatable, $stderr, 2, 2) { "Module '#{self}' is deprecated." }
      end
    end
    
    def method_deprecated_since(year, month, replacement_method_symbol_or_message = nil, send_instance_target = self, *incoming_args)
      depth_to_method = if send_instance_target == self
        0
      else
        1
      end
      
      calling_method = /`([^']*)'/.match(caller[depth_to_method])[1]
      
      already_notified = if calling_method
        calling_method = calling_method.intern
        @@deprecated_warned_symbol ||= {}
          
        if @@deprecated_warned_symbol[calling_method]
          true
        else
          @@deprecated_warned_symbol[calling_method] = true
          false
        end
      else
        puts 'Note: no method name retrieved.'
        false
      end
      
      if !already_notified
        if send_instance_target == self
          log_class = self
        else
          log_class = send_instance_target.class
        end
        
        caller_info = caller[2]
        
        elapsed_since_deprecation = Time.now - Time.local(year, month)
        
        if fatal_deprecation?(elapsed_since_deprecation)
          if replacement_method_symbol_or_message.kind_of?(Symbol)
            raise "The function is deprecated. Update source to refer to '#{replacement_method_symbol_or_message}' immediately."
          elsif replacement_method_symbol_or_message.kind_of?(String)
            raise "The function is deprecated. (#{replacement_method_symbol_or_message})"
          else
            raise "The function is deprecated. Remove references immediately."
          end
        elsif warn_deprecation?(elapsed_since_deprecation)
          if replacement_method_symbol_or_message.kind_of?(Symbol)
            ModuleLogging.do_log(Deprecatable, Deprecatable, $stderr, 2, depth_to_method + 2) { 
              "This function is deprecated, please substitute '#{replacement_method_symbol_or_message}'. (#{caller_info})" 
            }
          elsif replacement_method_symbol_or_message.kind_of?(String)
            ModuleLogging.do_log(Deprecatable, Deprecatable, $stderr, 2, depth_to_method + 2) { 
              "#{calling_method}: this function is deprecated. (#{replacement_method_symbol_or_message}, #{caller_info})" 
            }
          else
            ModuleLogging.do_log(Deprecatable, Deprecatable, $stderr, 2, depth_to_method + 2) { 
              "#{calling_method}: this function is deprecated. (#{caller_info})" 
            }
          end
        end
      end
      
      if send_instance_target && replacement_method_symbol_or_message.kind_of?(Symbol)
        send_instance_target.send(replacement_method_symbol_or_message, *incoming_args)
      end
    end
    
    def deprecation_time
      @deprecation_time
    end
    
    def deprecation_warning_period
      @deprecation_warning_period ||= 0
    end
    
    def deprecation_warning_period=(days)
      @deprecation_warning_period = days * 24 * 60 * 60
    end

    def deprecation_fatal_period
      @deprecation_fatal_period ||= 90 * 24 * 60 * 60
    end
    
    def deprecation_fatal_period=(days)
      @deprecation_fatal_period = days * 24 * 60 * 60
    end 
    
    def warn_deprecation?(elapsed_seconds)
      elapsed_seconds > deprecation_warning_period()
    end
    
    def fatal_deprecation?(elapsed_seconds)
      elapsed_seconds > deprecation_fatal_period()
    end
  end
  
  extend(ClassMethods)
  
  # Mark a function deprecated since the given year and month.
  # If the current time is past the warning deprecation time, a warning is given and the method is called as normal.
  # If a replacement method is given, it is called with the incoming args.
  # If the fatal deprecation time has passed, then an exception is raised. 
  def deprecated_since(year, month, replacement_method_symbol_or_message = nil, *incoming_args)
    Deprecatable.method_deprecated_since(year, month, replacement_method_symbol_or_message, self, *incoming_args)
  end
end