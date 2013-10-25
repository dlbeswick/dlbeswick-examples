require 'std/algorithm'
require 'std/module_logging'

module EnumerableInstances
  include ModuleLogging
  
  @debug = false
  
  def EnumerableInstances.debug=(b)
    @debug = b
  end
  
  def EnumerableInstances.finalize(weakref, classObj)
    Proc.new do
      classObj.weakrefInstances.delete(weakref)
    end
  end
  
  def self.addInheritedInstance(targetModule, instance)
    added = false
    weakref = instance.object_id
    
    instance.enumerableinstances_weakref = weakref

    # Ancestors must be searched for includes of EnumerableInstances to support recording instances of
    # modules that include EnumerableInstances. 
    # This function is only called by Module.extend or when a class is instantiated and there's no callback for 
    # when a module is "instantiated" by its inclusion in an instantiated class, so module instances cannot 
    # be recorded unless the ancestors are walked and these modules are found.
    # (test_module_instances)
    Algorithm.each_ancestor(targetModule, true) do |a|
      break if a == EnumerableInstances
      
      if a.respond_to?(:weakrefInstances) && a.weakrefInstances != nil
        added = true
        
        if @debug
          raise "Weakref #{weakref} already added." if a.weakrefInstances.include?(weakref)
        end
        
        a.weakrefInstances[weakref] = nil
        
        ObjectSpace.define_finalizer(instance, EnumerableInstances.finalize(weakref, a))

        # Any time a class is defined, a 'new' function is overridden and used to call this function.
        # If this test succeeds, it means that a class including EnumerableInstances has added an instance
        # of itself or a derived class, and so walking of the ancestor list will terminate before it reaches
        # any base classes that also include EnumerableInstances. Although those base classes must still record
        # a reference, that will be acheived by the 'super' call in the 'new' override. Walking any further
        # here will cause problems because the base class would add two instances of a derived class.
        # (test_late_base_multiple_inclusion, test_derived_inclusion)
        break if a.kind_of?(Class)
      end
    end

    raise "Could not find method 'weakrefInstances' in any ancestor." if !added
  end
  
  def self.propagate_to(receiver)
    if receiver.kind_of?(Class)
      method_chain = "enumerableinstances_old_new_#{receiver.name.gsub(':','_')}"
      
      receiver.module_eval <<-EOS
        class << self
          # Note: some lightweight classes don't have 'new' defined, such as Fixnum. It's undesireable to include
          # this functionality in them anyway. It's not a fatal error because EnumerableInstance could be included
          # indirectly through classes such as Viewable.
          if method_defined?(:new)
            # Apparently 'new' does not call user-overridden 'allocate' functions.
            # Only an explicit call to 'allocate' executes this function. 
            def allocate
              newObj = super
              
              EnumerableInstances.addInheritedInstance(#{receiver}, newObj)
      
              newObj
            end
            
            # Method chaining must be used instead of 'super' here. 
            # If 'new' is aliased to another method, then 'super' will call the 'new' function instead of the alias.
            # This problem arises with the Date class. See test_EnumerableInstances.rb::test_date_yaml
            alias_method :#{method_chain}, :new
            def new(*args, &block)
              newObj = begin
                #{method_chain}(*args, &block)
              rescue ::Exception=>e
                # Re-raise exception with less confusing backtrace (otherwise backtrace leads here, in eval code)
                e.set_backtrace(e.backtrace)
                raise
              end
              
              EnumerableInstances.addInheritedInstance(#{receiver}, newObj)
      
              newObj
            end
          end
        end
      EOS
    else
      receiver.module_eval <<-EOS
        # This function is necessary to ensure that modules can be included in other modules and still enable
        # enumerable instances.
        Algorithm.alias_class_method(self, :enumerableinstances_old_propagate_included, :included)
        def self.included(receiver)
          enumerableinstances_old_propagate_included(receiver)
  
          EnumerableInstances.propagate_to(receiver)
        end
        
        Algorithm.alias_class_method(self, :enumerableinstances_old_propagate_extended, :extended)
        def self.extended(receiver)
          enumerableinstances_old_propagate_extended(receiver)
          
          EnumerableInstances.addInheritedInstance(self, receiver)
        end
      EOS
    end
  end

  def self.is_included_in_module?(mod)
    if PlatformRuby.v19?
      mod.methods(false).include?(:weakrefInstances)
    else
      mod.methods(false).include?("weakrefInstances")
    end
  end
  
  Algorithm.alias_class_method(self, :enumerableinstances_old_included, :included)
	def self.included(receiver)
    if is_included_in_module?(receiver)
      $stderr.puts "Module #{receiver} has already included #{self}. Recursion occurs if this happens. The most common cause is inconsistent 'require' statements of this module in the codebase, please make sure every 'require' instance for module '#{receiver}' uses the same text."
      $stderr.puts caller
      return
    end
    
    enumerableinstances_old_included(receiver)
    
		receiver.module_eval(
		<<-STR
	  @weakrefInstances = {}
			
      def self.instances
        if !@weakrefInstances
          nil
        else
          @weakrefInstances.keys.collect do |i|
            ObjectSpace._id2ref(i)
          end
        end
      end

      def self.each_instance(&block)
        found = false
        
        Algorithm.each_ancestor(self, true) do |a|
          if a.respond_to?(:instances) && a.instances
            found = true
            a.instances.each(&block)
          end
        end
      
        raise "No method 'instances' found in any ancestor for class '#{self}')" if !found
      end
      
			def self.weakrefInstances
				@weakrefInstances
			end
		STR
		)
    
    EnumerableInstances.propagate_to(receiver)
	end

  def enumerableinstances_weakref=(o)
  	@enumerableinstances_weakref = o
	end
	
	# Find the nearest root class and return the instances of that class.
	def instances
		Algorithm.each_ancestor_singleton(self) do |a|
			if a.respond_to?(:instances) && a.instances
				return a.instances.dup
			end
		end

		raise "No method 'instances' found in any ancestor for '#{self}' (class: #{self.class})"
	end
	
	# Find the nearest root class and return all instances of that class besides the current instance.
	def other_instances
		instances.reject { |i| i == self }
	end
	
  # This function walks the ancestor list and returns the first list of instances that it finds.
  # This behaviour is necessary to allow classes derived from those including EnumerableInstances to respond
  # to the 'instances' call correctly, otherwise, only those classed directly including EnumerableInstances
  # could list their instances.
	def weakrefInstances
		Algorithm.each_ancestor_singleton(self) do |a|
			if a.respond_to?(:weakrefInstances)
				result = a.weakrefInstances
				if result != nil
					return result
				end
			end
		end

		raise "No method 'weakrefInstances' found in any ancestor for '#{self}' (class: #{self.class})"
	end

	def remove_instance
		raise "Object has not been added." if !@enumerableinstances_weakref
		self.weakrefInstances.delete(@enumerableinstances_weakref)
	end
end

module EnumerableInstancesWx
  def self.included(receiver)
    super

    receiver.module_eval(
    <<-STR
      def self.instances
        if !@weakrefInstances
          nil
        else
          result = []
          
          @weakrefInstances.each_key do |i|
            obj = ObjectSpace._id2ref(i)
            begin
              obj.sizer
              result << obj
            rescue ObjectPreviouslyDeleted
              EnumerableInstances.log { "Wx enumerable instance was deleted (" + obj.to_s + ")" }
              obj.remove_instance
            end
          end
          
          result
        end
      end
    STR
    )
  end
end