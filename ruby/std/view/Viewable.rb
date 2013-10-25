require 'std/AttributeReference'
require 'std/algorithm'
require 'std/ConfigYAML'
require 'std/deprecatable'
require 'std/IncludableClassMethods'
require 'std/module_logging'
require 'std/notify'
require 'std/raise'
require 'zlib'

# Class allowing modification of the way viewable dependencies are recorded for a certain class.
# Objects should return an instance of this class in response to :viewable_dependency_id.
class ViewableDependencyRecordSpecialization
  include Abstract
  include Comparable
  
  def initialize(dependant)
    abstract
  end

  # Should return the object instance that the id refers to.  
  def resolve
    abstract
  end
  
  def hash
    abstract
  end
  
  # This method must be implemented too.
  #def <=>(other)
  #end
end

# Viewable
# Interface for objects that can be the target of a View.
#
# If an object is interested in knowing when a Viewable instance is modified, it should mark itself as
# a dependency of the Viewable instance by calling viewable.viewable_dependant. The interested object
# will receive a call to a method called onViewableDependencyEvent.
#
# Many view classes enumerate information about the attributes of their view targets. Viewable also 
# includes functionality to support this.
# 
# Use the 'default_view' class method to nominate a certain view for an attribute.
# 'no_view' will explicitly declare that by default, a view of the class will not show that attribute.
module Viewable
  include IncludableClassMethods
	include ModuleLogging
  include NotifyHost
  include Raise
  
  self.log = false

	attr_reader :views

  module ClassMethods
    def default_view(symbol, viewClassOrString)
      @viewable_symbolToDefaultViewClass ||= {}
      @viewable_symbolToDefaultViewClass[symbol] = viewClassOrString
    end
    
    def no_view(symbol)
      @viewable_excludedAttributes ||= []
      if PlatformRuby.v19?
        @viewable_excludedAttributes << "@#{symbol}".intern
      else
        @viewable_excludedAttributes << "@#{symbol}"
      end
    end
    
    def viewable_excludedAttributes
      @viewable_excludedAttributes
    end
  
    def viewable_symbolToDefaultViewClass
      @viewable_symbolToDefaultViewClass
    end
  end

	class NotifyNameChange < Notify::Base
		def onChange
			@callback.call
		end
  end

  class ModificationRecord
    attr_accessor :who
    attr_reader :instigator
    
    def initialize(instigator)
      @instigator = instigator
      @who = nil
    end
  end

  def self.dependant_to_masters_entry?(dependant)
    @dependant_to_masters && @dependant_to_masters.has_key?(Viewable.get_dependency_id(object))
  end
  
  # used to prevent multiple finalizers being added to an object
  def self.dependant_hash_entry_global?(object)
    @dependant_hash_global ||= {}
    @dependant_hash_global.has_key?(Viewable.get_dependency_id(object))
  end
  
  def self.master_finalized_proc(master_id)
    proc { master_finalized(master_id) }
  end
  
  def self.get_dependency_id(dependant)
    responds_to = if dependant.respond_to?(:no_drb_respond_to?)
      dependant.no_drb_respond_to?(:viewable_dependency_id)
    else
      dependant.respond_to?(:viewable_dependency_id)
    end
      
    if responds_to
      dependant.viewable_dependency_id
    else
      dependant.object_id
    end
  end
  
  def self.dependant_added(master, dependant)
    log {
      dependant_class = if dependant.respond_to?(:viewable_class)
        dependant.viewable_class
      else
        dependant.class
      end 
      
      "Adding dependant-to-masters hash entry for dependant '#{Viewable.get_dependency_id(dependant)}' (#{dependant_class}), master '#{master.object_id}' (#{master.class})" 
    }
      
    master_id = master.object_id
    dependant_id = Viewable.get_dependency_id(dependant)
    
    @dependant_to_masters ||= {}
    hash = @dependant_to_masters[dependant_id] ||= {}
    hash[master_id] = true
    
    @master_to_masters_hash ||= {}
    array = @master_to_masters_hash[master_id] ||= [] 
    array << [hash, dependant_id]
    
    @with_master_finalizer ||= {}
    if !@with_master_finalizer.has_key?(master_id)
      log { "Adding finalizer for master with id '#{master_id}'." }
      @with_master_finalizer[master_id] = true
      ObjectSpace.define_finalizer(master, master_finalized_proc(master_id))
    end
    
    dependant.viewable_make_dependant_finalizer_if_required(Viewable.viewable_remove_dependencies_proc(dependant_id))
  end
  
  def self.dependant_to_masters(dependant_id)
    return [] if !@dependant_to_masters
      
    masters = @dependant_to_masters.fetch(dependant_id, {})
    
    result = []
      
    masters.each_key do |id|
      begin
        object = if id.respond_to?(:resolve)
          id.resolve
        else
          ObjectSpace._id2ref(id)
        end
        
        result << object
      end
    end
    
    result
  end
  
  def self.has_dependant_finalizer?(object)
    @with_dependant_finalizer && @with_dependant_finalizer[Viewable.get_dependency_id(object)] != nil
  end
  
  def self.has_dependant_finalizer(object)
    @with_dependant_finalizer ||= {}
    @with_dependant_finalizer[Viewable.get_dependency_id(object)] != nil
  end
  
  def self.master_finalized(master_id)
    Viewable.log { "Finalizing master with id '#{master_id}'." }
    
    mappings = @master_to_masters_hash[master_id]
    
    return if !mappings # tbd: master_finalized is being called multiple times. why? 
    
    mappings.each do |element|
      hash = element.first
      dependant_id = element.last
      
      if hash.delete(master_id) != nil
        Viewable.log { "  Master '#{master_id}' removed from a dependant-to-masters hash for dependant '#{dependant_id}'." }
      else
        raise "Master '#{master_id}' should have been removed from a dependant-to-masters hash for dependant '#{dependant_id}', but wasn't."
      end
    end

    @master_to_masters_hash.delete(master_id)
    @with_master_finalizer.delete(master_id) 
  end
  
  def self.remove_dependencies(object)
    log {
      object_class = if object.respond_to?(:viewable_class)
        object.viewable_class
      else
        object.class
      end 
      
      "Removing all dependencies referencing '#{object_class}' (#{Viewable.get_dependency_id(object)})." 
    }
    
    remove_dependencies_id(Viewable.get_dependency_id(object))
  end

  def self.remove_dependencies_id(objectID)
    Viewable.log { "Removing all dependencies added by (#{objectID})." }
    dependant_to_masters(objectID).each do |i|
      ids = i._viewable_dependant_hash
      if ids.delete(objectID) != nil
        Viewable.log { "  Removed from '#{i.class}' (#{Viewable.get_dependency_id(i)})." }
      end
    end
    
    @with_dependant_finalizer.delete(objectID) if @with_dependant_finalizer # shouldn't ever happen, here for debugging
  end
  
  def viewable_dependant?(object)
    _viewable_dependant_hash().has_key?(Viewable.get_dependency_id(object))
  end
  
  def self.viewable_remove_dependencies_proc(dependant_id)
    proc { remove_dependencies_id(dependant_id) }
  end

  def viewable_supports_dependencies?
    # returns false for types that don't use the standard method of instancing in ruby.
    # for example, True and False and not instanced. all 'true' and 'false' occurrances point to the same
    # two instances. those types of objects shouldn't be modified on a per-instance basis, because they're
    # not really instantiated. 
    # tbd: remove string class from here? they don't fit the above pattern.
    return !viewable_kind_of?(Numeric) \
      && !viewable_kind_of?(NilClass) \
      && !viewable_kind_of?(TrueClass) \
      && !viewable_kind_of?(String) \
      && !viewable_kind_of?(FalseClass)
  end
  
  def viewable_dependant(object)
    Kernel.raise "Object of type '#{self.class}' doesn't support dependencies." if !viewable_supports_dependencies?()
    Kernel.raise "Object of type '#{object.class}' must respond to 'onViewableDependencyEvent' to be added as a dependant." if !object.respond_to?(:onViewableDependencyEvent)
    Kernel.raise "Viewable: Object of type '#{object.class}' was added twice to object of type '#{self.class}'" if viewable_dependant?(object)
    Kernel.raise "Viewable: Object of type '#{object.class}' cannot depend on itself." if object.equal?(self)
    Kernel.raise "Viewable: Circular dependency created between object of type '#{object.class}' and object of type '#{self.class}'" if object.kind_of?(Viewable) && object.viewable_dependant?(object)

    Viewable.log { "Adding dependant '#{Viewable.get_dependency_id(object)}' (#{object.object_id}) to object ref '#{object_id()}'" }
    _viewable_dependant_hash()[Viewable.get_dependency_id(object)] = true

    Viewable.dependant_added(self, object)    
  end
  
  alias :viewable_addDependant :viewable_dependant

  def viewable_remove_dependant(object)
    viewable_dependants().delete(Viewable.get_dependency_id(object))
  end

  def viewableName
    deprecated_since(2009, 7, :viewable_name)
    viewable_name
  end
  
	# Human readable display name of the object.
	def viewable_name
    if to_s().length < 64 && to_s()[0..1] != '#<'
      to_s()
    else
	    /^\w*?\:*(\w+)$/.match(self.class.name)[1]
    end
	end
	
	# tbd: deprecated, remove this
	def viewableDescription
    deprecated_since(2009, 7, :viewable_description)
    viewable_description
	end
	
  # Additional description of the object.
	def viewable_description
	  ''
	end
	
	def viewable_summary
	  description = viewable_description
	  if !description.empty?
	    "#{viewable_name()} (#{viewable_description()})"
	  else
      viewable_name()
	  end
	end
	
  def modified(instigatorOrModificationRecord)
    record = if !instigatorOrModificationRecord.kind_of?(ModificationRecord)
      newRecord = ModificationRecord.new(instigatorOrModificationRecord)
      newRecord
    else
      instigatorOrModificationRecord
    end
      
    record.who = self
    
    Viewable.log { "#{self.class} modified by #{record.instigator.class}." }
    
    # New objects may be created during the modify event. These objects may add this object as a dependency of themselves.
    # This could lead to infinite loops or performing duplicate operations, so only iterate over the objects present
    # at the start of processing.
    _viewable_dependant_hash().keys.each do |i|
      begin
        object = begin
          if i.respond_to?(:resolve)
            i.resolve
          else
            ObjectSpace._id2ref(i)
          end
        rescue RangeError
          raise "Object with id '#{i}' encountered RangeError during _id2ref. This shouldn't happen, the finalizer should have been called."
        end
    
        # An object may not respond to 'onViewableDependencyEvent' even though they are listed as
        # a dependant. For instance, an object not including 'Viewable' could contain viewable attributes,
        # and another Viewable object could view it using 'View.viewAttribute'.
        if object.respond_to?(:onViewableDependencyEvent)
          # drb
          is_equal = if object.respond_to?(:drb_equal?)
            object.drb_equal?(record.instigator)
          else
            object.equal?(record.instigator)
          end
            
          # Don't send 'onViewableDependencyEvent' to the object that instigated the event.
          if !is_equal
            Viewable.log {
              class_name = if object.respond_to?(:viewable_class)
                object.viewable_class
              else
                object.class
              end
               
              "#{self.class} notify dependant #{class_name} (#{record.class}, instigator #{record.instigator.viewable_class})" 
            }
              
            object.onViewableDependencyEvent(record)
          else
            Viewable.log { 
              class_name = if object.respond_to?(:viewable_class)
                object.viewable_class
              else
                object.class
              end
              
              "#{self.class} skipping dependant #{class_name} as it was the instigator." 
            }
          end
        end
      end
    end
    
    record
  end

  def onViewableDependencyEvent(record)
    modified(record.clone)
  end

  def viewableAttributes?
    viewableAttributes().empty?
  end
  
	def viewableAttributes
		result = self.instance_variables - 
		  [:@viewable_dependants, :@unique_id]

		# discard attributes without accessors
	  result.delete_if do |str| 
	    !respond_to?(str[1..-1]) 
	  end
		
		# tbd: all viewable objects should have the opportunity to define metadata describing their attributes,
		# which is currently partially implemented by SymbolDefaults. 
		# that will remove the need for this check.
		if self.class.respond_to?(:inferred_attributes)
		  additions = self.class.inferred_attributes
      # discard inferred attributes that have already been marked by enumerating the instance variables. 
		  additions.delete_if { |string| result.include?(string) }
		  result += additions
		end
		
		# hack: explicitly include symbols tagged for default_view, even if no instance variable exists for
		# the symbol. In that case, the method matching the symbol will be called by Views::AllAttributes. 
		# this was initially to support viewing of UniqueRegistry objects.
		if self.class.viewable_symbolToDefaultViewClass && !self.class.viewable_symbolToDefaultViewClass.empty?
		  additions = self.class.viewable_symbolToDefaultViewClass.keys
		  
		  # discard default_view symbols that have already been marked by enumerating the instance variables. 
		  additions.delete_if { |symbol| result.include?("@#{symbol}".intern) }
		    
		  additions.collect! do |string|
		    if instance_variable_defined?("@#{string}")
          "@#{string}".intern
		    else
		      string.intern
		    end
		  end
		  
	    result += additions
		end
		
    result -= viewable_excludedAttributes()
    
		result
  end

  def viewable_clean_verify?
    true
  end
  
  def viewable_debuginfo
    "'#{self.class.name}'"
  end
  
  def viewable_dependants
    result = []
      
    _viewable_dependant_hash().keys.each do |i|
      begin
        result << if i.respond_to?(:resolve)
          i.resolve
        else
          ObjectSpace._id2ref(i)
        end
      rescue RangeError
      raise "Object with id '#{i}' encountered RangeError during _id2ref. This shouldn't happen, the finalizer should have been called."
      end
    end
    
    result
  end

  def _viewable_dependant_hash
    begin
      @viewable_dependant_hash ||= {}
    rescue TypeError=>e
      raise "'#{self}' is frozen."
    end
  end
  
  def viewable_dependant_ids
    _viewable_dependant_hash().keys
  end

  def viewable_dirtyTestReference
    begin
      if respond_to?(:to_yaml_content)
        Zlib.crc32(to_yaml_content())
      elsif respond_to?(:to_yaml)
        Zlib.crc32(to_yaml())
      else
        raise "Error, Viewable objects must respond to to_yaml_content or to_yaml"
      end
    rescue ::TypeError
      xx 'peepiss'
      puts self
      # if to_yaml didn't work for some reason (procs, anonymous classes) then use Marshal.
      # this could potentially cause undesirable behaviour. Not all internal state changes should 
      # "dirty" a view.
      note { "Please note: TypeError while attempting to form dirtyTestReference for object of type '#{target.viewable_class}'. Marshalling..." } 
      Zlib.crc32(Marshal.dump(self))
    rescue ::Exception=>e
      return true
      puts '+++POOPOO++++++++++'
      xx 'poopoo'
      puts self
      puts e
      puts e.backtrace
      raise
      puts '+++END+++++++'
    end
  end

  def viewable_equivalent(dirtyTestReference)
    start = Time.now
    
    test = self.viewable_dirtyTestReference
    equivalent = test == dirtyTestReference
    
    duration = Time.now - start
    if duration > 3
      err { "Viewable of type '#{self.class}' is taking too long to perform viewable_equivalent (#{duration} secs.) Check viewable_dirtyTestReference performance.\n#{test}" }
    end
    
    equivalent
  end
  
  def viewable_excludedAttributes
    result = []
    
    Algorithm.each_ancestor_kindof(self, Viewable) do |a|
      excluded = a.viewable_excludedAttributes
      if excluded
        result = result | excluded
      end
    end
    
    result
  end

  def viewable_defaultViewClassForSymbol(symbol)
    Algorithm.each_ancestor_kindof(self, Viewable) do |a|
      symbolToDefaultViewClasses = a.viewable_symbolToDefaultViewClass
      if symbolToDefaultViewClasses
        result = symbolToDefaultViewClasses[symbol]
        
        if result.kind_of?(String)
          result = eval(result)
        end
        
        return result if result
      end
    end
    
    nil
  end
  
  def viewable_ensure
    true
  end
  
  def viewable_class
    self.class
  end

  # viewable_kind_of? can fail when cclass is a class not present on the server. To avoid needing
  # the data system (server) to know all classes used by the view system, the class is retrieved from the
  # server but the comparison is done on the view system. The view system is much more likely to
  # have knowledge of more classes than the data system.
  # viewable_kind_of? is a more convenient call. This call is usually more correct, though.
  def self.viewable_kind_of?(instance, cclass)
    instance_class = instance.viewable_class
    instance_class <= cclass
  end
    
  def viewable_kind_of?(cclass)
    if cclass.kind_of?(String)
      # hack, drb doesn't marshal nested classes properly
      cclass = eval(cclass)
    end
    
    kind_of?(cclass)
  end
  
  def views_possible(subclasses = nil)
    subclasses ||= View.subclasses
    View.allPossibleFor(self, subclasses)
  end

  def viewable_on_attribute_view(attribute_reference)
  end
end

class Fixnum
  include Viewable
end

class Float
	include Viewable
end

class String
  include Viewable
end

class Array
	include Viewable
	
	def viewable_name
	  'Array'
	end
	
	def viewable_description
	  ''
	end
end

class Object
  # It's necessary to implement this functionality by adding a method to Object 
  # because of the need to support drb. DRb must have a method to call on an instance, otherwise
  # implementation is more complicated in this case.
  def viewable_make_dependant_finalizer_if_required(finalizer_proc)
    if !Viewable.has_dependant_finalizer?(self)
      ObjectSpace.define_finalizer(self, finalizer_proc)
    end
  end
end

class ArrayModificationRecord < Viewable::ModificationRecord
  attr_reader :indexes
  attr_reader :action
  
  # valid actions: 'add', 'remove'
  # 'replace': the instance at the given index was replaced with a new object instance
  # tbd: replace strings with class references
  def initialize(instigator, indexes, action)
    super(instigator)
    
    raise "Nil index present in indexes parameter." if indexes.include?(nil)
    
    @indexes = indexes
    @action = action
  end
  
  def drb_data
    super + [@indexes, @action]
  end
end

class TrueClass
  include Viewable

  # do nothing -- there's no use in detecting changes to the bool classes, they're basically immutable
  def viewable_dependant(obj)
  end
  
  def viewable_remove_dependant(obj)
  end
end

class FalseClass
  include Viewable
  
  # do nothing -- there's no use in detecting changes to the bool classes, they're basically immutable
  def viewable_dependant(obj)
  end
  
  def viewable_remove_dependant(obj)
  end
end

class NilClass
  include Viewable
  
  # do nothing -- the nil class is immutable and no changes will be made to it
  def viewable_dependant(obj)
  end
  
  def viewable_remove_dependant(obj)
  end
end

class Hash
	include Viewable
end

class Date
  include Viewable
end

class Range
  include Viewable
end

class AttributeReference::Base
	include Viewable
  
  def viewable_dirtyTestReference
    obj = get()
  
    raise "AttributeReference target must include Viewable." if !obj.viewable_kind_of?(Viewable)
    
    if obj != nil  
      obj.viewable_dirtyTestReference
    else
      nil
    end
  end

  def viewable_equivalent(rhs)
    Viewable.log { "Equivalence test for #{self.class} with target #{get().class}" }
    obj = get()
  
    if obj != nil  
      get().viewable_equivalent(rhs)
    else
      rhs == nil
    end
  end

  def viewable_ensure
    if get() && !get().respond_to?(:viewable_ensure)
      Viewable.raise("Target is not viewable. Detail: #{viewable_debuginfo()}")
    end
  end
  
  def viewable_debuginfo
    result = super
    
    hash = viewable_debuginfo_extra_hash()
    
    if !hash.empty?
      result += " ("
    
      first = true
      hash.each do |k,v|
        if !first
          result += ", "
        end
        
        result += "#{k}: #{v}"
        first = false
      end
      
      result += ")"
    end
    
    result
  end
  
  def viewable_name
    obj = get()

    if !obj.nil?
      obj.viewable_name
    else
      ''
    end
  end
  
  def viewable_description
    obj = get()

    if !obj.nil?
      obj.viewable_description
    else
      ''
    end
  end
  
protected
  def viewable_debuginfo_extra_hash
    hash = {}
      
    obj = get()
    if obj
      hash['attribute target'] = obj.viewable_debuginfo
    end
    
    hash
  end
end

class AttributeReference::AttributeSymbol
protected
  def viewable_debuginfo_extra_hash
    hash = super
    hash['symbol'] = @symbol.to_s
    hash
  end
end

class AttributeReference::ArrayElement
  def ensure_host_is_array
    raise "ArrayElements must be hosted by Arrays" if !host().viewable_kind_of?(Array)
  end
  
  def modified(instigatorOrModificationRecord)
    record = super
    # tbd: if no host is given, then perhaps the host modification needs to be moved to the 'set' call, which
    # is less desireable as it breaks the pattern used elsewhere throughout the code where a modification is
    # always followed by a manual call to 'modified'.
    raise "tbd: No host given -- investigate. Is this class ever useful without a stored host?" if !host()
    host().modified(ArrayModificationRecord.new(record.instigator, [idx()], 'replace')) if host()
  end
end

require 'std/mapping' #tbd: move to ViewableMapping class
class Mapping::Base
  include Viewable
end
