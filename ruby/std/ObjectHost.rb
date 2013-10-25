require 'std/ConfigYAML'
require 'std/IncludableClassMethods'
require 'std/UniqueIDConfig'

# Classes that include ObjectHost are able to declare ownership of a number of objects.
# This module often interacts closely with UniqueIDConfig. See that module's documentation for more details.
#
# Use the attribute call 'host' in the including class to designate that the given attribute contains an instance
# or array of instances of hosted objects.
module ObjectHost
	include ConfigYAML
	include IncludableClassMethods

	persist_by_method :hostedObjects
	
	class HostedObjects < Array
	  # += won't work correctly if this is removed
    def + (array)
      result = self.dup
      array.each { |e| result << e }
      result
    end
    
		def to_yaml(opts={})
      YAML::quick_emit( nil, opts ) do |out|
        out.seq( taguri, to_yaml_style() ) do |seq|
          each() do |i|
            raise "Hosted object of type '#{i.class}' must include UniqueIDConfig." if !i.respond_to?(:to_yaml_content)
            seq.add(UniqueIDConfig::ContentWriter.new(i))
          end
        end
      end
		end
	end
	
  module ClassMethods
    def host(symbol)
      @hosted_attributes ||= []
      @hosted_attributes << ('@'+symbol.to_s).intern
      persistent(symbol)
    end
    
    def hosted_attributes
      @hosted_attributes ||= []
    end
    
    def attribute_hosted?(symbol)
      @hosted_attributes.include?("@#{symbol}".intern)
    end
  end
  
 	def hostedObjects
 		result = HostedObjects.new
 		
    Algorithm.each_ancestor_kindof(self.class, ObjectHost, true) do |c|
      c.hosted_attributes.each do |sym|
        result += Array(instance_variable_get(sym))
	    end
    end
 		
 		result
 	end
 	
protected 	
  def loaded
    super
    # hostedObjects is not required after load, so clear it.
    # otherwise, viewable objects can experience problems.
    # tbd: eliminate the need for hostedObjects altogether.
    # it's currently required only because of quirks in Config and UniqueIDConfig. 
    remove_instance_variable(:@hostedObjects)
  end
end

# As per ObjectHost, but also provides the means to designate any individual object as hosted via the 'host' method.
module ObjectHostExplicit
  include ObjectHost
  
  def host(object)
    @hostedObjects ||= HostedObjects.new
    
    newObject = if object.respond_to?(:entries)
      object.entries.each do |a|
        if !a.kind_of?(Array)
          @hostedObjects << a
        else
          @hostedObjects << a[1]
        end
      end
    else
      object
    end

    raise "Object is already hosted." if @hostedObjects.include?(newObject)
    
    @hostedObjects << newObject
  end
  
  def hosted?(obj)
    @hostedObjects.find(obj) != nil
  end
  
  def hostedObjects
    @hostedObjects ||= HostedObjects.new
    super + @hostedObjects
  end
  
  def hostedObjects=(obj)
    if obj.kind_of?(Array)
      @hostedObjects = HostedObjects.new
      @hostedObjects.concat(obj)
    elsif obj.kind_of?(HostedObjects)
      @hostedObjects = obj
    else
      raise "Tried to assign an object of type '#{obj.class}' to hostedObjects."
    end
  end
  
  def unhost(object=nil)
    @hostedObjects ||= HostedObjects.new
    if object
      raise "Attempt to remove object from non-owning host" if !@hostedObjects.include?(object)
      @hostedObjects.delete(object)
    else
      @hostedObjects.clear
    end
  end
end
