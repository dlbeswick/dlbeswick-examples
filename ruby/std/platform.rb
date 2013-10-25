require 'std/abstract'
require 'std/algorithm'
require 'std/module_logging'

class Platform
  include Abstract
  extend Comparable
  
  def self.setup(classSym, moduleObj=Object)
    begin
      newClass = Class.new(moduleObj.const_get(platform_name_for_module(classSym)))
    rescue NameError=>e
      puts "Platform Error: #{name}: No platform specific class for #{classSym} has been defined."
      raise
    end
    
    moduleObj.const_set(classSym, newClass)
  end

  def self.calculate_platform
    raise "Plaform-derived classes must implement 'self.calculate_platform'. Or did you call a method on Platform, rather than the derived class?"
  end
    
  def self.platform
    @platform ||= calculate_platform()
    @platform
  end
  
  def self.platform=(name)
    @platform = name
  end
  
  def self.platform_class_postfix
    platform()
  end

  def self.commonNameForModule(name)
    name.to_s + "Common"
  end
  
  def self.platform_name_for_module(name)
    raise "Platform not defined" if self.platform.empty?
    name.to_s + self.platform
  end

  def self.specific
    raise "#{caller[0]}: This function's implementation is platform-specific."
  end
  
  def self.method_missing(symbol)
    if /\?/ =~ symbol.to_s 
      self.platform.downcase == symbol.to_s[0..-2]
    else
      super
    end
  end
end

module PlatformSpecific
  include Abstract
  
  def self.included(mod)
    raise "It is only valid to include this module in another module." if !mod.kind_of?(Module)
    
    mod.instance_eval <<-EOS
      def self.platform_class
        if !@platform_class 
          match = /(.+)Specific/.match(name())
          
          if !match
            raise "Please name your class in the format <Platform-derived class name>Specific, or return a Platform-derived class from an override of this method."
          end

          platform_class_name = match[1]
          
          begin          
            @platform_class = Object.const_get(platform_class_name)
          rescue NameError
            raise "Couldn't find a class named '" + platform_class_name + "'"
          end
        end
        
        @platform_class
      end
      
      def self.included(mod)
        class_postfix = platform_class().platform_class_postfix
        
        match = /.+?(\#\{class_postfix\})/.match(mod.name)
        
        if match != nil
          commonClassName = mod.name[0...match.begin(1)] 
    
          unscopedClassName = /(.+\:\:)+?(.+)/.match(commonClassName)
          if unscopedClassName
            unscopedClassName = unscopedClassName[2]
          else
            unscopedClassName = commonClassName
          end
          
          # tbd: will only work for one depth of nested module
          containingModule = /(.+)(\:\:)/.match(mod.name)
          
          if !containingModule
            containingModule = Object
          else
            containingModule = Object.module_eval(containingModule[1])
          end
          
          # create a constant with a platform-agnostic name and assign the including module to it 
          containingModule.const_set(unscopedClassName.intern, mod)
        else
          # Class is unsuitable for this platform or has had a platform-agnostic name created, 
          # so remove from enumerable subclass list    
          mod.instance_eval("include Abstract")
        end
      end
    EOS
  end
end

class PlatformRuby < Platform
  def self.v18?
    !v19?
  end
  
  def self.v19?
    RUBY_VERSION >= "1.9"
  end

  def self.calculate_platform
    if PlatformRuby.v19?
      '19'
    else
      '18'
    end
  end
  
  def self.program_name
    Path.new($0).file.no_extension.literal
  end
end

module PlatformRubySpecific
  include PlatformSpecific
end