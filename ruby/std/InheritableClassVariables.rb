require 'std/algorithm'

# Creates a class instance variable that can be overridden in derived classes.
# 
# Ordinary class instance variables are not accessible from derived classes. 
# Ruby class variables only allow one value for the variable across all members of the 
# class hierarchy.
# This module allows for both of those behaviours.
#
# InheritableClassVariables.classmethods_attr_inherit can be called from within 
# IncludableClassMethods' "ClassMethods" blocks, which allows inheritable class variables 
# to be propagated to classes that include that module.
module InheritableClassVariables
  include IncludableClassMethods
  
  module ClassMethods
    def attr_inherit(symbol)
      InheritableClassVariables.attr_inherit(ClassMethods, symbol, false)
    end
  end
  
  def self.classmethods_attr_inherit(mod, symbol)
    attr_inherit(mod, symbol, false)
  end
  
  def self.attr_inherit(mod, symbol, do_instance_eval)
    eval_text = <<-EOS
      def #{symbol}=(value)
        @#{symbol} = value
      end
      
      def #{symbol}
        Algorithm.each_ancestor(self, true) do |ancestor|
          if ancestor.instance_variable_defined?(:@#{symbol})
            return ancestor.instance_variable_get(:@#{symbol})
          end
        end
        
        nil
      end
    EOS
    
    if do_instance_eval
      mod.instance_eval(eval_text)
    else
      mod.module_eval(eval_text)
    end
  end
end
