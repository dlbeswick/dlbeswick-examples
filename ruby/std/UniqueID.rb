require 'std/IncludableClassMethods'
require 'std/module_logging'
require 'std/raise'

# Extends a class to assign unique IDs to object instances, and allows retrieval of object
# instances by ID.
#
# It's possible to use any kind of unique id, but an integer id is the default.
# To do so, override the uniqueID instance method and validID? class method.
#
# TBD: abstract auto id logic out of this module further and add a UniqueIDAuto module.   
module UniqueID
  include IncludableClassMethods
  include ModuleLogging
  include Raise
  
  self.log = false

  UNBOUND_UNIQUE_ID = Object.new
  def UNBOUND_UNIQUE_ID.to_s
    "<UNBOUND>"
  end

  UNBOUND_UNIQUE_ID.freeze
  
  module ClassMethods
    def auto_id
      @auto_id ||= 0
    end
    
    def bind_unique_id(object, id)
      note { "Bind #{id} to #{object}" }
      UniqueID.raise "Invalid id: #{id}" if !valid_id?(id)

      native_registry = object.uniqueid_context
      
      if native_registry.has_key?(id)
        UniqueID.raise "ID '#{id}' is already in use. (assigned object class: #{object_for_unique_id(id, object.uniqueid_context).class}, incoming: #{object.class})"
      end
      
      if object.has_id?
        note { "Unbinding object of type '#{object.class}' with id '#{object.unique_id}'" }
        object.uniqueid_unbind
      end
      
      object.instance_variable_set(:@unique_id, id)
      
      value = registry_value_for(object)
      
      native_registry[id] = value 
      
      UniqueID.log { "Object of type '#{object.class}' bound to id '#{id}' with value '#{value}'" }
    end
    
    def id_mapped?(id, id_context)
      id_context.has_key?(id)
    end
  
    # careful with this method
    def auto_id=(number)
      @auto_id = number
    end
    
    # Return a map of ids to object descriptions
    def uniqueid_report(context = UniqueID.global_registry())
      result = {}
        
      context.each do |k,v|
        result[k] = objectForID(k).class.to_s
      end
      
      result
    end
    
    def object_for_unique_id(id, uniqueid_context, absence_fatal = true)
      found_object_id = uniqueid_context[id]
      
      if found_object_id
        begin
          ObjectSpace._id2ref(found_object_id)
        rescue RangeError
          raise "No native reference found for ID: #{id} (native #{found_object_id})"
        end
      else
        if absence_fatal
          raise "No object registered for ID: #{id}"
        else
          nil
        end
      end
    end
    
    def report_dupes
      @report_dupes ||= true
    end
    
    def registry_value_for(object)
      object.object_id
    end

    def report_dupes=(b)
      @report_dupes = b
    end
    
    def uniqueid_unbind_id(id, id_context)
      object = object_for_unique_id(id, id_context)
      raise "An object could not be found for id '#{id}'" if object.nil?
      id_context.delete(id)
      object.instance_variable_set(:@unique_id, UNBOUND_UNIQUE_ID)
    end
    
    def valid_id?(id)
      id.kind_of?(Integer)
    end
  end
  
  extend(ClassMethods)
	
  def bind_id(id)
    self.class.bind_unique_id(self, id)
  end
  
	def equal_unique?(rhs)
	  if rhs.respond_to?(:unique_id)
	    unique_id() != rhs.unique_id
	  else
	    raise "Can't compare a UniqueID object to an object not of kind UniqueID."
	  end
	end
	
  def ensure_id
    self.unique_id
  end
  
	def id
	  unique_id()
	end
	
	def recalculate_unique_id
    new_id = generate_unique_id
		raise "Bad id" if new_id == nil
		self.class.bind_unique_id(self, new_id)
	end

	def has_id?
    @unique_id != nil
	end

	# for debugging only
	def report_dupes?
    UniqueID.report_dupes || @uniqueid_report_dupes 
	end

  def uniqueid_bound?
    has_id? && uniqueid_context().has_key?(@unique_id)
  end

  def unique_id
    integer_auto_id
  end
  
  def uniqueid_unbind
    if !uniqueid_bound?
      raise "Unbind requested, but object is not bound."
    end
    
    self.class.uniqueid_unbind_id(unique_id(), uniqueid_context())
  end
	
  def uniqueid_context
    UniqueID.global_registry
  end
  
  def self.global_registry
    @native_registry ||= {}
  end

protected
	def integer_auto_id
    if !@unique_id
      recalculate_unique_id
      @uniqueid_report_dupes = self.class.report_dupes()
    end
    
    @unique_id
	end

	def generate_unique_id
		id = nil
		
		begin
			id = UniqueID.auto_id
      UniqueID.auto_id = id.next 
		end while self.class.id_mapped?(id, UniqueID.global_registry)
		
		id
	end	
end
