require 'std/abstract'
require 'std/AttributeReference'
require 'std/deprecatable'
require 'std/IncludableClassMethods'
require 'std/module_logging'
require 'std/path'
require 'std/raise'
require 'std/SymbolDefaults'

module DLB
  module Config
    include Abstract
    include Deprecatable
    include IncludableClassMethods
    include ModuleLogging
    include Raise
    include SymbolDefaults # SymbolDefaults can be considered integral to Config functionality.
  
    # Thrown when config loading fails.
    # A variety of exceptions could be raised from a number of classes during loading, so in that case
    # all exceptions are caught and wrapped in this exception class.
    # "caught_exception" contains the original exception that was caught, if any.
    class ExceptionLoad < Exception
      attr_reader :caught_exception
      
      def initialize(caught_exception)
        @caught_exception = caught_exception
        super()
      end
      
      def to_s
        if @caught_exception
          "Config load: #{@caught_exception.message} (#{@caught_exception.class})\n\nBacktrace:\n#{@caught_exception.backtrace.join("\n")}"
        else
          super
        end
      end
    end
    
  	module ClassMethods
  	  def config_version
  	    0
  	  end
  	  
      def config_upgrade(new, old, old_version)
        raise "Object requires upgrade, but no upgrade method has been defined (#{self}, version #{old.config_version(self)} -> #{config_version()}.)"
      end
      
  	  # TBD: remove nonFatal parameter and always raise exceptions.
  		def load(target=nil, str=nil, nonFatal=true, variables=nil)
  			newObj = nil
  			
  			begin
  				if !str
  					path = if target
  						target.configPath
  					else
  						configPath
  					end
  					
  					Config.log { "Opening #{path}..." }
  					  
  					if path.file_size == 0
  					  Config.raise "File #{path} is empty."
  					end
  					
  					str = File.new(path.to_sys).read
  				end
  				
  				target._pre_perform_load if target	
  				newObj = performLoad(str)
  				
          if !target
            target = newObj
          end
          
          if !variables
            variables = target.configVariables
          end
          
          #id must be bound to target before try_resolve_references, but target must also be filled
          #at that point so that unique line 223 doesn't trigger. target is only filled afted configpostloadtransform,
          #but at that point references must be resolved so that config upgrading will succeed.
          #must fix this problem.
          
          newObj.config_process_unresolved(nil, target)
          
          id = newObj.unique_id if newObj.kind_of?(UniqueIDConfig)
          
          if !target.equal?(newObj)
            if newObj.kind_of?(UniqueIDConfig) && newObj.class.id_mapped?(id, newObj.uniqueid_context)
              newObj.uniqueid_unbind

              # I had to add this line around the time of commit c2f858b41edf41b6b64db98819de32ceaf884941
              # It would fail with 'can't unbind because object not bound'
              # It wasn't 'bound', but unique_id was set, probably due to code in config_process_unresolved.
              # I think the 'can't unbind' check got tougher. It's ok to clear this here.
              target.instance_variable_set(:@unique_id,nil)

              target.bind_id(id)
            end
          end
          
          UniqueIDConfig.try_resolve_references
  
          target.config_on_created_during_load
          transformed = target.configPostLoadTransform(newObj, nil, nil)
  
          raise "configPostLoadTransform should return itself in this context." if transformed != target
  			rescue SystemCallError=>e
          raise ExceptionLoad.new(e) if !nonFatal
  				return nil
        rescue NoMethodError=>e
          raise e
        rescue RuntimeError=>e
          raise e
  			rescue Exception=>e
  			  # tbd: fix, this hides errors!
  			  $stderr.puts "Exception while attempting to load Config object. Raising ExceptionLoad."
          $stderr.puts "Target class is '#{target.class}'" if target
          $stderr.puts e.class
          $stderr.puts e
          $stderr.puts e.backtrace
          raise ExceptionLoad.new(e)
  			end
  	
  			# hack: return newObj, because overriding base types like Array won't have configVariables defined
  			newObj
  		end
  		
  		def performLoad(str)
  		end
  		
  		def elementName=(symbol)
  		end
  		
  		# tbd: check if 'host' parameter still has purpose.
  		def persistent(symbol, host=nil)
          # freeze to prevent accidental modification, clients sometimes try to set 'host'.
          add_config_variable(AttributeReference::Instance.new(symbol, host).freeze)
          self.configVariablesDirty = true
  		end
  		
  		# As for 'persistent', but uses an instance method to retrieve the value rather than the instance variable.
      def persist_by_method(symbol, host=nil)
        # freeze to prevent accidental modification, clients sometimes try to set 'host'.
        add_config_variable(AttributeReference::ReaderInstanceSet.new(symbol, host).freeze)
        self.configVariablesDirty = true
      end
      
      # Use this helper function to define both a persistent variable and an reader attribute.
      # Note that config functions have the ability to read and write to the variable, regardless of the
      # attribute's access restrictions. 
      def persistent_reader(symbol)
        persistent(symbol)
        attr_reader symbol
      end
  	
      # Use this helper function to define both a persistent variable and a writer attribute. 
      # Note that config functions have the ability to read and write to the variable, regardless of the
      # attribute's access restrictions. 
      def persistent_writer(symbol)
        persistent(symbol)
        attr_writer symbol
      end
    
      # Use this helper function to define both a persistent variable and an accessor attribute.
  		def persistent_accessor(symbol, host=nil)
        persistent(symbol, host)
        
  			if method_defined?("#{symbol}=") && method_defined?("#{symbol}")
          puts caller
          puts "DEPRECATED: phase out the dual use of attr_accessor and persistent_accessor."
        else
          attr_accessor symbol
        end
  		end
  	
  		def configElementName
  			self.name
  		end
  	
      def config_override_element_name
        self.name
      end
      
  		def configExists?
  		  deprecated_since(2009,07,:config_exists?)
  		end
      
  		def config_exists?
  			configPath.exists?
  		end
  		
  		def configExtension
  			"ini"
  		end
  		
  		def configFilename
  			Path.new(configElementName).sub_invalid
  		end
  	
  		def configPath
  			path = Config.basePath + configFilename
  			path.extension = configExtension
  			path
  		end
  		
  		def configVariables
  			if self.configVariablesDirty
          @configVariableList = self.configVariableList.sort_by { |obj| obj.symbol.to_s }
  				self.configVariablesDirty = false
  			end
  
  			@configVariableList
  		end
  		
      def should_write_config_variable?(attribute_name, instance, is_cloning)
        !inferred_value?(attribute_name, instance) &&
        (!is_cloning || config_var_should_clone?(attribute_name))
      end
      
      # a list of strings indicating loaded variables that may be present in a config file, but shouldn't
      # be propagated to the restored instances as instance variables.   
      # tbd: build this list with proper attribute 'no_restore :symbol' method calls 
      def config_vars_no_restore
        # tbd: move non-Config specific strings to subclasses
        ['unique_id', 'class', 'config_version', 'viewable_dependants', 'viewable_dependant_hash', 'viewable_dependency_id']
      end
      
      # a list of strings indicating loaded variables that shouldn't be transferred to cloned objects.
      # tbd: build this list with proper attribute 'no_clone :symbol' method calls 
      def config_vars_no_clone
        ['unique_id']
      end
      
      def config_var_should_clone?(var_name)
        !config_vars_no_clone().include?(var_name)
      end
    
  	protected
      def add_config_variable(attribute_reference)
        list = configVariableList()
        
        if !list.include?(attribute_reference)
          list << attribute_reference
        end
      end
        
      def configVariableList
  			if !@configVariableList
  				@configVariableList = []
  			  if kind_of?(Class)
  					inheritFromAncestors
  				end
  			end
  			
  			@configVariableList
  		end
  		
  		def configVariablesDirty=(o)
  			@configVariablesDirty = o
  		end
  			
  		def configVariablesDirty
  			@configVariablesDirty = true if !@configVariablesDirty
  			@configVariablesDirty
  		end
  		
  		def inheritConfigVariables(ancestor)
  			if ancestor.respond_to?(:configVariableList)
  				@configVariableList ||= []
  				@configVariableList = @configVariableList | ancestor.configVariableList
  			end
  		end
  		
  		def inheritFromAncestors(ancestorMod = self)
      	ancestorMod.ancestors.each do |ancestor|
      		next if ancestor == ancestorMod
  
      		inheritFromAncestors(ancestor)
  
      		ancestorMod.inheritConfigVariables(ancestor) if ancestorMod.respond_to?(:inheritConfigVariables)
      	end
  		end
  	end
  
  	def self.basePath
  		return Path.new('./Config')
  	end
  	
    # This is a unique per-instance identifier available to config methods.
  	# Different config systems may interpret it differently. Usually, it's used to
  	# determine the filename used to store settings for the instance.
  	# This default implementation must be overridden if the instance using the module
  	# is not a singleton instance.
  	def configElementName
  		self.class.configElementName
  	end
  	
    # Not currently used by the Config class, but may be in future. For use by client classes.
    def configElementNamePerObject
      configElementName
    end
  
    def configExists?
      deprecated_since(2009,07,:config_exists?)
    end
  
    def config_exists?
  		configPath.exists?
  	end
  	
  	def configExtension
  		self.class.configExtension
  	end
  	
  	def configFilename
  		Path.new(configElementName).sub_invalid
  	end
  
  	# Not currently used by the Config class, but may be in future. For use by client classes.
    def configFilenamePerObject
      Path.new(configElementNamePerObject).sub_invalid
    end
    
  	def configBasePath
  		Config.basePath
  	end
  
  	def configPath
  		path = configBasePath + configFilename
  		path.extension = configExtension
  		path
  	end
  	
    # Not currently used by the Config class, but may be in future. For use by client classes.
    def configPathPerObject
      path = (configBasePath + configFilenamePerObject)
      path.extension = configExtension
      path
    end
    
  	def configNeedsTransform?
  		return @configNeedsTransform != nil && @configNeedsTransform == true
  	end
  	
  	def _configNeedsTransform=(b)
  		@configNeedsTransform = b
  	end
  	
  	# Fills the current object with data from the config file.
    # If 'str' is given, then the contents of that string are parsed as a config file. Otherwise, the result of
  	# 'configPath' is loaded and parsed.
  	# Throws Config::Exception if config loading fails due to a missing config file, or for any other reason.
  	# tbd: replace 'str' with an IO object
  	def load(str=nil)
  		self.class.load(self, str, false)
  	end
  	
  	def configCopyCommon(target)
  	  target_variables = target.configVariables
  	  
  	  configVariables().each do |v|
  	  	if (target_variables.detect { |o| o.symbol == v.symbol }) != nil
          next if !self.class.should_write_config_variable?(v.symbol, self, true)
          
  	  		obj = v.get(self)
  	  		
  	  		begin
  	  		  # this check is for objects of class Unique
            v.set(obj, target)
  	  		rescue TypeError, NoMethodError
  					v.set(Marshal.load(Marshal.dump(obj)), target)
  				end
  			end
  		end
  		
  		target
    end
  
    # this method should be called whenever a Config object is created as a result of loading
    def config_on_created_during_load
      self._configNeedsTransform = true
    end
    
    # Iterates through all config variables and calls 'config_process_unresolved_references' on them if
    # possible.
    # Clients of Config should propagate the method call to any unresolved id objects, as well as to any
    # other objects that may contain unresolved references. 
    def config_process_unresolved(attribute_reference, host)
      variables = configVariables()
      
      variables.each do |v|
        symbol_instance_name = '@'+v.symbol.to_s
        obj = instance_variable_get(symbol_instance_name)
        
        if obj.respond_to?(:config_process_unresolved)
          obj.config_process_unresolved(v, self)
        end
      end
    end
    
    # This method provides the opportunity to manipulate the object after its config has been loaded, but
    # before the incoming loaded object is assigned to the loading instance.
    # The incoming loaded object is supplied in the 'source' parameter.
    # This default implementation calls configPostLoadTransform recursively on persistent variables, 
    # sets defaults where required, and calls 'set' on persistent variables to assign their values to that of
    # the incoming loaded object.  
  	def configPostLoadTransform(source, attribute_reference, host)
      _configPostLoadTransform(source, attribute_reference, host)
  	end
  	
    def _configPostLoadTransform(source, attribute_reference, host)
      return self if !self.configNeedsTransform? || !self.respond_to?(:configVariables)
      
      self._configNeedsTransform = false
      
      variables = configVariables()
      
      variables.each do |v|
        note { "#{self.class}.#{v.symbol}" }
        symbolInstanceName = '@'+v.symbol.to_s
        obj = source.instance_variable_get(symbolInstanceName)
        
        if obj.respond_to?(:configPostLoadTransform)
          obj = obj.configPostLoadTransform(obj, v, self)
        end
      
        v.set(obj, self)
      end
      
      config_resolve_versioning(source)
      
      variables.each do |v|
        # this must go after versioning, or spurious errors can be generated when new versions of objects
        # add config variables.
        config_set_defaults(v)
      end
      
      loaded()
      
      self
  	end
    
  	def config_resolve_versioning(incoming)
  	  Algorithm.each_superclass(incoming.class) do |c|
        incoming_version = incoming.config_version(c)
        
  	    current_version = if c.respond_to?(:config_version)
  	      c.config_version
  	    else
  	      0
  	    end
  	    
  	    if incoming_version < current_version
  	      c.config_upgrade(self, incoming, incoming_version)
  	    end
  	  end
  	end
  	
  	def config_set_defaults(attribute_reference)
      v = attribute_reference
      
      # TBD: check for nil in case variable was added to class but absent from config. 
      # needs a better fix (check class presence but config absence)
      #
      # If the source object was created by YAML, then the initialize function wouldn't have been called.
      # This means that if a member was added to the class since it was saved, then it will end up 'nil' even
      # though the member may be set in 'initialize'.
      # If the member under consideration has not been loaded (currently this means 'nil') then SymbolDefaults
      # is used if possible.  
      if v.get(self).nil?
        has_symbol_default = respond_to?(:defaultObjForSymbol?) && defaultObjForSymbol?(v.symbol)
  
        if has_symbol_default
          default_obj = defaultObjForSymbol(v.symbol) 
          v.set(default_obj, self)
          return true
        else
          Config.err { "#{self.class}.#{v.symbol} had no data defined in its hosts' config." } 
          
          if !respond_to?(:defaultObjForSymbol?)
            Config.err { "  The host does not respond to 'defaultObjForSymbol?'." }
          else
            Config.err { "  The host includes SymbolDefaults, but has no default symbol entry of that name." }
          end
        end
        
        false
      end
  	end
  
  	def config_upgrade_instance_variable_removed(symbol)
      if instance_variable_defined?(symbol)
        if instance_variable_get(symbol).respond_to?(:unresolved_disregard)
          instance_variable_get(symbol).unresolved_disregard
        end
        
        remove_instance_variable(symbol)
      end
  	end
  
  	def configVariables
      if self.class.respond_to?(:configVariables)
        return self.class.configVariables()
      end
      
  		Algorithm.each_ancestor(self.class) do |c|
  			if c.respond_to?(:configVariables)
  				return c.configVariables
  			end
  		end
  		
  		raise "No configVariables found"
  	end
  	
  	def config_version(query_class)
      validate_config_version(query_class)
  	  abstract # override this function in subclasses and call 'validate_config_version'
  	end
  	
  	def configDefaultFor(symbol)
  		Algorithm.each_ancestor(self.class, true) do |c|
  			if c.respond_to?(:configDefaultFor)
  				return c.configDefaultFor(symbol)
  			end
  		end
  		
  		raise "No configDefaultFor found"
  	end
  	
  	def save(path=nil)
  		path = configPath if !path
  		path.make_dirs
  
      begin
        io = StringIO.new
        perform_save(io)
        
        path.open("w") do |f|
    			c = self.class
    			f.write(io.string)
    			Config.log { "Wrote #{path}" }
    	  end
    	rescue SystemCallError=>e
        Config.log { "Save failed for #{path}: #{e}" } 
      end
  	end
  
  	def save_stream(stream)
  		perform_save(stream)
  	end
  
    def to_configstr
      io = StringIO.new
      save_stream(io)
      io.string
    end
  
    def _pre_perform_load
    end
    
  protected
    # returns true if the object has the given symbol defined in its config file.
    def config_hasEntryFor?(symbol)
      raise "Requires implementation."
    end
  
    def validate_config_version(query_class)
      raise "query_class must be one of this class instance's ancestors (#{self.class} has ancestors #{self.class.ancestors()}" if !self.class.ancestors().include?(query_class)
    end
    
  	def configTarget
  		self
  	end
  	
    # loaded is called during configPostLoadTransform, after transform of all variables is complete but before
  	# defaults are assigned. 
  	# References to UniqueID objects will not be resolved. tbd: incorporate a 'partially-resolved' uniqueID ref?
  	# This change is new, and was introduced because id binding of top-level objects undergoing config.load was
  	# not operating correctly.
  	# This change may cause previously working code to fail if it accessed object references during the 'loaded'
  	# function.
  	# Alternative: call loaded only after object resolution is complete. 
  	def loaded
  	end
  	
  	def perform_save(stream)
  	end
  end
end

# standard lib support
class Array
  def config_process_unresolved(attribute_reference, host)
    self.each_with_index do |e, i|
      if e.respond_to?(:config_process_unresolved)
        e.config_process_unresolved(AttributeReference::ArrayElement.new(i, self, attribute_reference), self)
      end
    end
    
    self
  end

  def configPostLoadTransform(source, attribute_reference, host)
		self.each_with_index do |e, i|
			if e.respond_to?(:configPostLoadTransform)
				self[i] = e.configPostLoadTransform(e, AttributeReference::ArrayElement.new(i, self, attribute_reference), self)
			end
		end
		
		self
	end
end

class Hash
  def config_process_unresolved(attribute_reference, host)
    self.each do |k, v|
      # There's an outstanding issue here -- this code doesn't support arrays of hashes. Is it right to assume that the attribute reference is of a symbol type?
      key_attribute = AttributeReference::HashKeyElement.new(k, attribute_reference.symbol, host)
      if k.respond_to?(:config_process_unresolved)
        k.config_process_unresolved(key_attribute, host)
      end

      if v.respond_to?(:config_process_unresolved)
        v.config_process_unresolved(AttributeReference::HashValueElement.new(key_attribute, host), host)
      end
    end
  end
  
	def configPostLoadTransform(source, attribute_reference, host)
		newHash = self.class.new
		
		self.each do |k, v|
		  key_attribute = AttributeReference::HashKeyElement.new(k, attribute_reference.symbol, host)
			if k.respond_to?(:configPostLoadTransform)
				k = k.configPostLoadTransform(k, key_attribute, host)
			end

			if v.respond_to?(:configPostLoadTransform)
				v = v.configPostLoadTransform(v, AttributeReference::HashValueElement.new(key_attribute, host), host)
			end
			
			newHash[k] = v
		end
		
		newHash
	end
end
