require 'std/algorithm'
require 'std/instance_exec'

module MethodChain
  def MethodChain.super(instance, super_info, *args)
    return if super_info.empty?
    current_method = super_info.pop
    args = [super_info] + args + current_method[2] 
    
    if current_method[1].arity > 0 && args.length != current_method[1].arity
      raise "The number of arguments in the call to 'super' doesn't match the number expected (#{current_method[1].arity} != #{args.length})."
    end
    
    instance.instance_exec(*args, &current_method[1])
  end
  
  def self.included(mod)
    mod.instance_eval do
      def self.methodchain_methods(symbol)
        result = []
          
        Algorithm.each_ancestor(self, true) do |a|
          if a.instance_variable_defined?(:@methodchain_methods)
            hash = a.instance_variable_get(:@methodchain_methods)
            result = hash[symbol] + result if hash.has_key?(symbol)
          end 
        end
        
        result
      end
      
      def self._chain(instigator_id, symbol, use_module_eval, is_module_method, *extra_block_args, &block)
        @methodchain_methods ||= {}
          
        if !@methodchain_methods.has_key?(symbol) && method_defined?(symbol)  
          raise "The method '#{symbol}' has been defined prior to this chaining request."
        end

        # prevent recursion by allowing only one chain call per class, per symbol and ignoring additional
        # requests.
        existing = methodchain_methods(symbol).find { |e| e[0] == instigator_id }
        return if existing  
        ary = @methodchain_methods[symbol] ||= []
        ary << [instigator_id, block, extra_block_args]
        
        eval_str = <<-EOS
          @methodchain_defining = true
        EOS
        
        if is_module_method
          eval_str << <<-EOS
            def self.#{symbol}(*args)
          EOS
        else
          eval_str << <<-EOS
            def #{symbol}(*args)
          EOS
        end
        
        eval_str << <<-EOS
            super_info = methodchain_methods(:#{symbol}).dup
            raise "#{self}: @methodchain_methods not found in any ancestor for symbol '#{symbol}'." if super_info.empty?
            current_method = super_info.pop
            args = [super_info] + args
            args += current_method[2] if current_method[2]
            instance_exec(*args, &current_method[1])
          end
          @methodchain_defining = false
        EOS
        
        if use_module_eval
          module_eval eval_str
        else
          instance_eval eval_str
        end
      end
            
      def self.module_chain(instigator_id, symbol, *block_args, &block)
        _chain(instigator_id, symbol, true, true, *block_args, &block)
      end

      #def self.chain(instigator, symbol, *block_args, &block)
      #  _chain(instigator, symbol, true, false, *block_args, &block)
      #end

      def self.method_added(symbol)
        if !@methodchain_defining &&
          class_variable_defined?(:@@methodchain_methods) &&
          @@methodchain_methods &&
          @@methodchain_methods.has_key?(symbol)
          raise "The method '#{symbol}' has already been chained in another module. Redefinition without chaining is not allowed."
        end 
      end
    end
  end
end