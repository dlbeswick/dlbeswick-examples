require 'std/UniqueID'
require 'std/abstract'
require 'std/ConfigYAML'
require 'std/EnumerableSubclasses'

# Include this module in a class along with ConfigYAML to enable saving and restoring object references 
# from config files.
#
# This module interacts with the ObjectHost module. ObjectHosts take ownership of objects through the 'host'
# instance method. The host is then responsible for writing the contents of its child UniqueIDConfig objects.
# Instances held in any other Config object are conceptually 'references' to the hosted object, and  
# are written out as such and resolved on load to the instances in their host.
#
# Redesign of this class and UniqueID is incomplete.
# They should form a base on which to create classes that manage disjoint sets of IDs.
# The code supporting a 'global' context should move out of this module into a new client module.
module UniqueIDConfig
  include EnumerableSubclasses
  include ConfigYAML
  include IncludableClassMethods
  include ModuleLogging
  include Raise
  include UniqueID
  
  self.log = false		
  
  attr_reader :saveContent
  
  module ClassMethods
    def add_unresolved_reference(unresolved)
      @unresolved ||= []
      @unresolved << unresolved
    end
    
    def remove_unresolved_reference(unresolved)
      @unresolved ||= []
      @unresolved.delete(unresolved)
    end
    
    def try_resolve_global_references(id_remapping_hash)
      if !@unresolved || @unresolved.empty?
        UniqueIDConfig.log { "#{self}: Nothing to resolve." }
      else
        UniqueIDConfig.log { "#{self}: Resolving references." }
        
        @unresolved.delete_if do |u|
          u.resolveIfPossible(id_remapping_hash)
        end
      end
    end
    
    def try_resolve_references(id_remapping_hash)
      try_resolve_global_references(id_remapping_hash)
    end
    
    def enumerate_unresolved_references(object, parent, parent_variable, visited = [])
      result = []
      
      visited << object
      
      if object.kind_of?(YAML::DomainType)
        err { "  YAML domain type encountered: #{object.inspect}" }
      elsif object.respond_to?(:configVariables)
        object.configVariables.each do |v|
          child = v.get(object)
          
          if child.kind_of?(Unresolved)
            result << [object, v.symbol]
          end
          
          if visited.find { |x| x.equal?(child) } == nil
            result += enumerate_unresolved_references(child, object, v, visited)
          end
        end
      elsif object.kind_of?(Array) || object.kind_of?(Hash)
        begin
          object.each do |e|
            if e.kind_of?(Unresolved)
              result << [parent, parent_variable.symbol]
            end
            
            if visited.find { |x| x.equal?(e) } == nil
              result += enumerate_unresolved_references(e, object, nil, visited)
            end
          end
        rescue TypeError
        end
      end
      
      visited.pop
      
      result
    end
    
    def uniqueidconfig_ensure_resolution(root_object)
      return if !@unresolved
      
      @unresolved.each do |u|
        UniqueIDConfig.err { "Unresolved: #{u} (oid #{u.object_id()})" }
      end
      
      if !@unresolved.empty?
        error_text = "#{@unresolved.length} unresolved references."
        err error_text
        err "Wait, discovering unresolved references for report..."
        
        if root_object
          unresolved = enumerate_unresolved_references(root_object, nil, nil)
          unresolved.each do |record|
            if record[1].kind_of?(AttributeReference::AttributeSymbol)
              puts "#{record[0].class}.#{record[1].symbol}"
            else
              puts "#{record[0].class}.#{record[1]}"
            end
          end
        end
        
        raise error_text
      end
    end
    
    def unresolved
      @unresolved
    end
  end
  
  class ContentWriter
    def initialize(obj)
      @obj = obj
    end
    
    def to_yaml(opts={})
      @obj.to_yaml_content(opts)
    end
  end
  
  def configElementName
    self.class.configElementName + unique_id().to_s
  end
  
  def _pre_perform_load
    super
    
    # the final object to which loaded config data will be assigned via configPostLoadTransform should have
    # its id temporarily unbound, so that the temporary object that receives loaded data can have the
    # correct final id assigned during load. the id will later be unbound from the temporary object and
    # re-assigned to the final object. 
    self.class.uniqueid_unbind_id(unique_id(), uniqueid_context())
  end
  
  def to_yaml_type
    "!david,2007/uniqueid"
  end

  def to_yaml(opts={})
    YAML::quick_emit( nil, opts ) do |out|
      out.map( taguri, to_yaml_style() ) do |map|
        map.add( 'unique_id', unique_id() )
      end
    end
  end

  def to_yaml_content(opts={}, is_cloning=false)
    result = YAML::quick_emit( nil, opts ) do |out|
      out.map( YAML::tagurize( "david,2007/configyaml_uniqueid_content" ), to_yaml_style() ) do |map|
        if self.class.should_write_config_variable?('unique_id', unique_id(), is_cloning)  
          map.add( 'unique_id', unique_id() )
        end
        
        outputYAML(map, is_cloning)
      end
    end
    
    result
  end
  
  def configClone
    # this generally won't work for anything but the most basic objects
    # to get this done properly, the resolution step must proceed after id recalculation and then must
    # enact a mapping from the old object id to the new one.
    newObj = ConfigYAML.newFromYAML(to_configstr_content(true))
    raise "Clone returned same object." if newObj.object_id == object_id()
    newObj.config_post_cloned(unique_id)
    newObj
  end

  # tbd: I don't think it's good practice to change the signature of overridden methods. 
  def config_post_cloned(old_id)
    ensure_id()
    config_process_unresolved(nil, self)
    UniqueIDConfig.try_resolve_references({old_id => unique_id()})
  end
  
  def perform_save(stream)
    stream.write(to_configstr_content)
  end
  
  def to_configstr_content(is_cloning=false)
    to_yaml_content({}, is_cloning)
  end

  extend(ClassMethods)  
  
  def self.ensure_resolution(root_object)
    uniqueidconfig_ensure_resolution(root_object)

    subclasses().each do |c|
      c.uniqueidconfig_ensure_resolution(root_object)
    end
  end
  
  def self.try_resolve_references(id_remapping_hash={})
    log { "Trying to resolving references with global context." }
    try_resolve_global_references(id_remapping_hash)

    log { "Trying to resolve references through all UniqueIDConfig subclasses." }
    subclasses().each do |c|
      c.try_resolve_references(id_remapping_hash)
    end
  end
  
  # as for UniqueID.bind, but resolves references afterwards
  #def bindID(id, allowCollision=false)
  #		super
  # This is currently commented out.
  # Resolution is typically performed during config loading.
  # However, vagaries in the system can mean that temporary objects can be bound to persistent ids if resolution
  # occurs here. Disabled for now, should remove in future. Resolution is occurring in Config after load.
  #UniqueIDConfig.tryResolveReferences
  #end
  
  class Unresolved
    include Abstract
    include Raise
    
    attr_reader :id
    attr_reader :host
    
    def initialize(id, unresolved_reference_class)
      @id = id
      UniqueIDConfig.log { "Adding unresolved reference id '#{id}' to class '#{unresolved_reference_class}'" }
      @unresolved_reference_class = unresolved_reference_class
      @unresolved_reference_class.add_unresolved_reference(self)
    end
    
    def initialize_copy
      raise "This object should not be duplicated."
    end
    
    def config_resolved?
      false
    end
    
    def configyaml_clear_load_map
      # This object has no need of a load map
    end
    
    def config_process_unresolved(attributeReference, host)
      raise "A host must be supplied." if !host
      @attributeReference = attributeReference
      @host = host
    end

    def inspect
      "Unresolved object id: #{@id}"
    end
    
    def resolveIfPossible(id_remapping_hash)
      abstract
      false
    end
    
    def resolve(id_remapping_hash={})
      raise "Failed to resolve reference: #{self}" if !resolveIfPossible(id_remapping_hash)
    end
    
    def to_yaml(opts={})
      raise "to_yaml invoked on unresolved object reference (#{inspect})"
      nil
    end
    
    def unresolved_disregard
      @unresolved_reference_class.remove_unresolved_reference(self)
    end
  end
  
  class UnresolvedGlobalID < Unresolved
    def initialize(id)
      raise "Attempt to construct #{self.class} with nil id." if id.nil?
      super(id, UniqueIDConfig)
    end
    
    def resolveIfPossible(id_remapping_hash)
      raise "Can't resolve with no host defined." if !@host
      
      remapped_id = id_remapping_hash[@id]
      final_id = if remapped_id != nil
                   remapped_id
                 else
                   @id
                 end 
      
      UniqueIDConfig.log { "Trying to resolve global reference with id '#{final_id}'" }
      if remapped_id != nil
        UniqueIDConfig.log { "  (note: id '#{@id}' was remapped to '#{final_id}')" }
      end
      
      if @attributeReference
        if UniqueID.id_mapped?(final_id, UniqueID.global_registry)
          @attributeReference.set(UniqueID.object_for_unique_id(final_id, UniqueID.global_registry), @host)
          
          if @attributeReference.get && !@attributeReference.get.config_resolved?
            raise "Reference still apparently unresolved after setting to a resolved object: is the AttributeReference class functioning? (#{@attributeReference})"
          end
          
          UniqueIDConfig.log { "  Ok." }
          true
        else
          UniqueIDConfig.log { "  ID not mapped." }
          false
        end
      else
        UniqueIDConfig.err { "  No attributeReference for unresolved global id '#{final_id}' (config_process_unresolved not called, oid #{object_id()}.)" }
        false
      end
    end
    
    def to_s
      "Unresolved global id #{@id.inspect}"
    end
  end

  def config_resolved?
    true
  end
  
  def uniqueidconfig_can_bind_on_load?(was_cloned)
    !was_cloned
  end
end

YAML::add_domain_type( "david,2007", "uniqueid" ) do |type, val|
  id = val['unique_id']
  
  if id.nil?
    raise "Error, uniqueid entry in config with nil unique_id value: #{val.inspect}"
  end
  
  UniqueIDConfig::UnresolvedGlobalID.new(id)
end

YAML::add_domain_type( "david,2007", "configyaml_uniqueid_content" ) do |type, val|
  objClass = eval(val['class'])
  id = val['unique_id']
  was_cloned = val['was_cloned']
  
  UniqueIDConfig.log { "Loading configyaml_uniqueid_content with class '#{objClass}' and id '#{id}'" }
  
  modified_loadmap = ConfigYAML.modified_loadmap(val)
  
  newObj = YAML.object_maker(objClass, modified_loadmap)
  newObj.config_on_created_during_load
  newObj.config_yamlLoadMap = val

  begin
    if newObj.uniqueidconfig_can_bind_on_load?(was_cloned)
      if !newObj.class.id_mapped?(id, newObj.uniqueid_context())
        newObj.bind_id(id)
      else
        existingObj = objClass.object_for_unique_id(id, newObj.uniqueid_context())
        if existingObj.report_dupes?
          UniqueIDConfig.raise "Unique ID clash -- id #{id} is assigned to '#{existingObj}', but bind is requested for object of type '#{objClass}'."
        end
      end
    end
  rescue Exception=>e
    UniqueIDConfig.err { "Error during load on object with ID '#{id}', class '#{objClass}'"}
    raise
  end

  newObj
end
