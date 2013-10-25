require 'std/algorithm'
require 'std/deprecatable'
require 'std/IncludableClassMethods'
require 'std/method_chain'
require 'std/module_logging'

# Allows data defining a default value for a given symbol.
#
# The attribute method "default_proc" accepts a symbol, and a block that should return the default object.
# The block is passed the initializing class instance.
#
# The method "default" allows clients to provide either a proc as above, or an object value that is then 
# cloned before assignment to the target instance variable.
#
# SymbolDefaults is propagated form base classes to derived classes, but not from included modules.
# Therefore, SymbolDefaults must be explicitly included in each module for which support is required.
#
# 'symboldefaults_init' can be called in a constructor to set instance variables to their given defaults.
# This technique avoids duplication of instance variable initialisation code in class constructors.
#
# If the user's intention is to set defaults for instance variables defined in each class in a hierarchy, then
# symboldefaults_init should be called in each class' initializer, after the call to 'super'. 
# This approach was chosen because it is usually undesirable to perform construction of derived classes
# before the base class is fully constructed.
#
# tbd: Consider renaming this class to 'AttributeDefaults' 
module SymbolDefaults
	include IncludableClassMethods
	include ModuleLogging
	include MethodChain

	SymbolDefaults.log = false
	
	# Call this function in the client class and derived classes instead of setting values explicitly in the
	# initializer. That will prevent duplication of code and information.
	# NOTE: I don't currently know a way to get the class in which the call was initially made.
	# What this function basically needs is support for 'non-virtual' function calls.
	# This function currently assumes that all classes in a SymbolDefaults hierarchy will call symboldefaults_init
	# and their superclass's initializer.
	#
	# The desired behaviour is for least-derived classes to initialize defaults, then next derived class, and so on.
	#  
  def symboldefaults_init(current_module)
    self.class.symboldefaults_set_all(self, current_module)
  end
  
	module ClassMethods
    # use this attribute for simple classes to create a default initializer
    def symboldefaults_init
      this_module = self
      define_method(:initialize) do
        super()
        symboldefaults_init(this_module)
      end
    end
    
    # Define a default for an instance variable.
    def default(symbol, value = nil, init = true, &block)
      if block
        setDefaultSymbolProc(symbol, init, &block)
      else
        setDefaultSymbolValue(symbol, value, init)
      end
    end

    # As with 'default', but also creates an accessor.
    def default_accessor(symbol, value = nil, init = true, &block)
      attr_accessor(symbol)
      default(symbol, value, init, &block)
    end
        
    # As with 'default', but also creates an reader.
    def default_reader(symbol, value = nil, init = true, &block)
      attr_reader(symbol)
      default(symbol, value, init, &block)
    end

    # As with 'default', but also creates a  write.
    def default_writer(symbol, value = nil, init = true, &block)
      attr_writer(symbol)
      default(symbol, value, init, &block)
    end
      
    # Define a default value, but do not apply the default during symboldefaults_init
    def default_noinit(symbol, value = nil, &block)
      default(symbol, value, false, &block)
    end
    
    def setDefaultSymbolProc(symbol, init = true, &block)
      @defaultSymbolProcs ||= {}
      @defaultSymbolProcs[symbol] = [init, block]
    end
    
    alias_method :default_proc, :setDefaultSymbolProc

    # Allows definition of a set of expected default values for this symbol.
    # The block is passed an object of the class that contains the symbol, and is called each time the
    # default choices are requested by the application.
    # The block should return an array of expected values for the symbol. 
    def default_choices(symbol, choices = nil, default_value = nil, &block)
      needs_accessor = if PlatformRuby.v18?
        if !instance_methods(false).include?(symbol.to_s) && !instance_methods(false).include?("#{symbol}=")
          attr_accessor(symbol)
        end
      else
        if !instance_methods(false).include?(symbol) && !instance_methods(false).include?("#{symbol}=".intern)
          attr_accessor(symbol)
        end
      end
      
      @default_symbol_choices ||= {}
      if !block
        if choices.kind_of?(Array)
          default_value ||= choices.first
          
          default(symbol, default_value)

          block = proc { choices }
        else
          raise "Must supply either a block or array."
        end
      end
      
      @default_symbol_choices[symbol] = block
    end

    def default_symbol_choices
      @default_symbol_choices
    end
    
    # If the given instance variable for the symbol is not nil, then the accessor returns that value.
    # Otherwise, the accessor executes the block and returns the result.
    #
    # Note: this is not the most correct place for this code, as it is not strictly involved with
    # attribute defaults. Consider an 'attribute metadata' module. 
    def attr_infer(symbol, value=nil, &block)
      block = proc { value } if !block
      
      attr_writer symbol
      
      class_var = inferred_attribute_classvar_name(symbol)
      
      class_variable_set(class_var, block)
      
      # note that 'inferred_value' is not called below for speed.
      module_eval <<-EOS
        def #{symbol}
          if !@#{symbol}.nil?
            @#{symbol}
          else
            instance_eval(&#{class_var})
          end
        end
        
        def _#{symbol}
          @#{symbol}
        end
      EOS
    end

    # Convenience
    def persistent_infer(symbol, value=nil, &block)
      persistent symbol
      attr_infer(symbol, &block)
    end
    
    # Define a 'model' value for a given attribute.
    # The data should always be valid within the context of the attribute's use, but otherwise can be any
    # chosen value.
    # An attribute's default value can be considered a model value, but the converse may not always be true.
    # The choice of model value should effectively communicate the required structure or format of the 
    # attribute's data.
    # The most typical use for this class is in defining a sample value for inferred attributes. The model
    # value can be retrieved for use in a GUI or any other place that example data may help a user. 
    def attr_model_value(symbol, value=nil, &block)
      # methods are used rather than data as an easy way to ensure that derived classes can override
      # model values for their attributes while still allowing both base and derived to return the
      # desired result.
      if value.nil? == false
        define_singleton_method("_symboldefaults_model_value_#{symbol}") do |instance|
          value.dup
        end
      else
        raise "A block must be given, or value specified." if !block
        define_singleton_method("_symboldefaults_model_value_#{symbol}") do |instance|
          instance.instance_eval(&block)
        end
      end
    end
    
    # Returns true if a model value can be inferred for the given symbol by one of several methods.
    def model_value?(instance, symbol)
      model_value_explicit?(symbol) \
      || inferred_attribute?(symbol) \
      || instance.defaultObjForSymbol?(symbol)
    end
    
    # Returns true if a model value is defined for the given symbol by a previous call to attr_model_value.
    def model_value_explicit?(symbol)
      respond_to?("_symboldefaults_model_value_#{symbol}")
    end
    
    def _model_value(instance, symbol)
      if model_value_explicit?(symbol)
        send("_symboldefaults_model_value_#{symbol}", instance)
      elsif inferred_attribute?(symbol)
        inferred_value(symbol, instance)
      else
        instance.defaultObjForSymbol(symbol)
      end
    end
    
    def inferred_attributes
      class_variables().find_all { |variable| (variable =~ /@@default_infer_/) != nil}.collect do |variable|
        variable.to_s.gsub("@@default_infer_", '@').intern
      end
    end
    
    def inferred_attribute?(symbol)
      class_variable_defined?(inferred_attribute_classvar_name(symbol))
    end
    
    def inferred_attribute_classvar_name(symbol)
      "@@default_infer_#{symbol}".intern
    end
    
    # Returns true if the value for the instance's variable is the inferred value.
    def inferred_value?(symbol, instance)
      inferred_attribute?(symbol) && 
        (!instance.instance_variable_defined?("@#{symbol}") || instance.instance_variable_get("@#{symbol}").nil?)
    end
    
    def inferred_value(symbol, instance)
      instance.instance_eval(&class_variable_get(inferred_attribute_classvar_name(symbol)))
    end
    
    def symbol_reset_to_inferred_value(symbol, instance)
      raise "Symbol is not inferred." if !inferred_attribute?(symbol)
      instance.instance_variable_set("@#{symbol}", nil)
    end

    def symboldefaults_choices_for(symbol)
      Algorithm.each_ancestor_kindof(self, SymbolDefaults, true) do |a|
        if a.default_symbol_choices
          result = a.default_symbol_choices[symbol]
          return result if result != nil
        end
      end
      
      nil
    end
    
    def block_has_args?(block)
      if RUBY_VERSION >= "1.9.1"
        block.arity != 0
      else
        block.arity != -1
      end
    end
    
    def symboldefaults_call_default_block(target, block)
      target.instance_eval(&block)
    end
    
    def defaultSymbolProcs
      @defaultSymbolProcs
    end
    
    def default_proc_for_symbol?(symbol)
      @defaultSymbolProcs && @defaultSymbolProcs.has_key?(symbol)
    end
    
    def defaultProcForSymbol(symbol)
      # search ancestors for a respons to 'defaultSymbolProcs'.
      # only return if the map returned from that function contains an entry for the requested symbol,
      # otherwise continue up the ancestor list.  
      # tbd: is it necessary to examine ancestors? maybe if modules include modules.
      Algorithm.each_ancestor_kindof(self, SymbolDefaults, true) do |a|
        if a.defaultSymbolProcs() && a.defaultSymbolProcs().has_key?(symbol)
          return a.defaultSymbolProcs()[symbol]
        end
      end
    
      nil
    end
    
    def symboldefaults_set_all(target, target_module)
      defaultSymbolProcs = target_module.instance_variable_get(:@defaultSymbolProcs)
      return if !defaultSymbolProcs
    
      defaultSymbolProcs.each do |k, v|
        if v[0] == true
          value = symboldefaults_call_default_block(target, v[1])
    
          target.instance_variable_set('@'+k.to_s, value)
        end
      end
    
      SymbolDefaults.log { "#{self} defaults set." }
    end
    
    def setDefaultSymbolValue(symbol, value, init = true)
      setDefaultSymbolProc(symbol, init) do 
        begin
          value.dup
        rescue TypeError
          value 
        end
      end
    end
  end

	def self.defaultTypeForInstance(object)
		if object.class <= Integer
			0
		elsif object.class <= Numeric
			0.0
		elsif object.class <= String
			''
	  else
      nil
		end
	end

	# This method is called when no proc was found for a given symbol.
	def defaultTypeFallback(object)
		SymbolDefaults.defaultTypeForInstance(object)
	end

	# Returns true if an appropriate object that could be considered a valid default for the 
	# given instance variable can be inferred from the information about the symbol.
	# See also the comments in defaultObjFromSymbol.
	def defaultObjForSymbol?(symbol)
    raise "Symbol expected, got '#{symbol.class}'" if !symbol.kind_of?(Symbol)

    # inferred values can always be set to nil.
    if self.class.inferred_value?(symbol, self)
      return true
    end
    
		Algorithm.each_ancestor_kindof(self, SymbolDefaults, true) do |a|
		  return true if a.defaultProcForSymbol(symbol) != nil
		end
		
    # no explicit default symbol entry was found.
    # try to construct a default object from the type of any existing instance of the symbol.
		object = if respond_to?(:symbol)
			send(:symbol)
		else
			instance_variable_get("@#{symbol}".intern)
		end
		
		defaultTypeFallback(object) != nil
	end
	
  # Tries to infer the most appropriate object that could be considered a valid default for the 
  # given symbol, and returns a new instance of that object. 
	# The logic here should be kept in sync with defaultObjForSymbol?
  # This method is different from model_value in that it is more implementation-focused.
  # For example, it considers 'nil' to be an appropriate default value for inferred attributes. 
	# This is appropriate for systems that are constructing the internals of objects, but not for 
	# systems that wish to present suitable model objects to users.
	def defaultObjForSymbol(symbol)
    raise "Symbol expected, got '#{symbol.class}'" if !symbol.kind_of?(Symbol)
    
    # inferred values can always be set to nil.
    if self.class.inferred_value?(symbol, self)
      return nil
    end
    
		Algorithm.each_ancestor_kindof(self, SymbolDefaults) do |a|
			symbolProc = a.defaultProcForSymbol(symbol)
			
      if symbolProc
        return self.class.symboldefaults_call_default_block(self, symbolProc[1])
      end
	  end
  
    # no explicit default symbol entry was found.
	  # try to construct a default object from the type of any existing instance of the symbol.
		object = if respond_to?(:symbol)
			send(:symbol)
		else
			instance_variable_get("@#{symbol}".intern)
		end
		
		defaultTypeFallback(object)
  end
  
  def default_choices_for_symbol?(symbol)
    Algorithm.each_ancestor_kindof(self, SymbolDefaults) do |a|
      return true if a.symboldefaults_choices_for(symbol) != nil
    end
    
    false
  end
  
  def default_choices_for_symbol(symbol)
    Algorithm.each_ancestor_kindof(self, SymbolDefaults) do |a|
      result = a.symboldefaults_choices_for(symbol)
      if result != nil
        result = instance_eval(&result) 
        return result ||= []
      end
    end
    
    []
  end

  def model_value?(symbol)
    self.class.model_value?(self, symbol)
  end
  
  def model_value(symbol)
    self.class._model_value(self, symbol)
  end
end
