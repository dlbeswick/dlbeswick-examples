require 'std/algorithm'
require 'std/method_chain'

# A module can include this module to allow it to define a set of class methods that will be
# transferred to an including module and client modules and classes, or an including class and its subclasses.
#
# Create a module called 'ClassMethods' within the definition of the including module. The instance methods
# defined in that module will be designated as class methods.
#
# Add an instance variable called '@included' in your 'ClassMethods' module to perform tasks
# when the class methods are added to a target class. The variable should contain a proc accepting the
# module for which methods have been added. 
module IncludableClassMethods
	def self.included(mod)
    mod.instance_eval { include MethodChain }
       
    if mod.kind_of?(Class)
      raise "Module '#{name()}' can only be included in another module (included in '#{mod}')."
    end
    
    mod.module_chain "IncludableClassMethods_#{mod}", :included do |super_info, target_mod_or_class|
      MethodChain.super(self, super_info, target_mod_or_class)
      
      IncludableClassMethods.give_classmethods(mod, target_mod_or_class)

      # if a second module includes the original module, then propagate IncludableClassMethods behaviour to
      # that second module, so that a class can ultimately be given the original class methods. 
      if !target_mod_or_class.kind_of?(Class)
        if !IncludableClassMethods.has_classmethods?(target_mod_or_class)
          classmethods_copy = Module.new
          classmethods_copy.module_eval("include #{name()}::ClassMethods")
          target_mod_or_class.const_set(:ClassMethods, classmethods_copy)
        else
          target_mod_or_class.const_get(:ClassMethods).module_eval("include #{name()}::ClassMethods")
        end 
        
        # Ensure the module will propagate the new ClassMethods to any client classes/modules.
        # TBD: write a test for this case
        target_mod_or_class.module_eval('include IncludableClassMethods')
      end
      
      # add the 'on included' block, if supplied
      classmethods = IncludableClassMethods.get_classmethods(mod)
      
      if classmethods.instance_variable_defined?(:@included)
        target_mod_or_class.instance_eval &classmethods.instance_variable_get(:@included)
      end
    end
  end

  def self.get_classmethods(source_mod)
    source_mod.const_get(:ClassMethods)
  end
    
  # Adds the methods in source_mod's ClassMethods module to the target module or class.
  # Further includes of the module won't propagate these class methods.
  def self.give_classmethods(source_mod, target_mod_or_class)
    if source_mod.kind_of?(Class)
      raise "Class methods can only be given to another module."
    end
    
    if !source_mod.const_defined?(:ClassMethods)
      raise "Module '#{self}' included IncludableClassMethods, but does not contain a 'ClassMethods' module within its definition." 
    end
    
    target_mod_or_class.extend(get_classmethods(source_mod))
  end
  
  if RUBY_VERSION >= "1.9"
    def IncludableClassMethods.has_classmethods?(mod)
      mod.const_defined?(:ClassMethods, false)
    end
  else
    def IncludableClassMethods.has_classmethods?(mod)
      mod.const_defined?(:ClassMethods)
    end
  end
end
