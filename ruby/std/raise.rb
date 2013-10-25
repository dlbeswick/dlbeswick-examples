require 'std/algorithm'

# Standardises exception behaviour across classes.
# Include Raise in any class that may raise its own exception.
#
# Calling 'raise' with a string argument will raises an exception of type Classname::Exception.
# Call 'exception_context' in your class definition to define a function to be called in this case.
# Pass either a symbol representing a function to be called, or a block accepting an object instance.
# The result of the function call or block is converted to a string and appended to the exception message.
# This can be useful for appending descriptive information to the class' exceptions, for example, a file
# name associated with the class instance of a picture file.
#
# Calling 'raise' with any other arguments results in raise's usual behaviour.
module Raise
  def self.included(mod)
    super
    
    mod.module_eval <<-EOS
      if !const_defined?(:Exception) # prevents 'superclass mismatch' errors when 'load' is used to reload script 
        if kind_of?(Class) && superclass.const_defined?(:Exception)
          class Exception < Exception
          end
        else
          class Exception < RuntimeError
          end
        end
      end
      
      def self.exception(*args)
        context_block = @raise_exception_module_context
        
        if context_block
          message = args[0]
          
          context = begin
            instance_eval(&context_block)
          rescue ::Exception => e
            'context retrieval failed: ' + e.to_s
          end
           
          message << " (" + context.to_s + ")"
          raise_exception_class().new(message)
        else
          raise_exception_class().new(*args)
        end
      end
      
      def self.raise(*args)
        if args[0].kind_of?(String)
          Kernel.raise(self, args[0], caller())
        else
          Kernel.raise(*args)
        end
      end
      
      def self.raise_exception_context
        @raise_exception_context
      end
      
      def self.raise_exception_class
        Exception
      end

      def self.exception_context(symbol=nil, &block)
        if block
          @raise_exception_context = block
        else
          @raise_exception_context = proc { send(symbol) }
        end
      end

      def self.exception_module_context(symbol=nil, &block)
        if block
          @raise_exception_module_context = block
        else
          @raise_exception_module_context = proc { send(symbol) }
        end
      end
    EOS
  end
  
  def exception(*args)
    context_block = raise_exception_context()
    
    if context_block
      message = args[0]
      
     context = begin
        instance_eval(&context_block)
      rescue ::Exception => e
        'context retrieval failed: ' + e.to_s
      end
       
      message << " (#{context})"
      raise_exception_class().new(message)
    else
      raise_exception_class().new(*args)
    end
  end
  
  def raise(*args)
    if args[0].kind_of?(String)
      Kernel.raise(self, args[0], caller())
    else
      Kernel.raise(*args)
    end
  end

  def raise_exception_class
    if self.class.respond_to?(:raise_exception_class)
      self.class.raise_exception_class
    else
      RuntimeError
    end
  end
  
  def raise_exception_context
    Algorithm.each_ancestor_kindof(self, Raise) do |m|
      if m.respond_to?(:raise_exception_context)
        result = m.raise_exception_context
        return result if result
      end
    end
    
    nil
  end
end
