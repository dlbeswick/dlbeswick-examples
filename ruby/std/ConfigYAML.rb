require 'std/Config'
require 'std/method_chain'
require 'yaml'
YAML::ENGINE.yamler='syck'

if PlatformRuby.v18? && Object.const_defined?(:ConfigYAML)
  $stderr.puts "Module #{"ConfigYAML"} seems to be already defined. Recursion can occur if this happens. The most common cause is inconsistent 'require' statements of this module in the codebase, please make sure every 'require' instance for this module uses the same text. Backtrace:"
  $stderr.puts caller
else

module ConfigYAML
  include DLB::Config
  include IncludableClassMethods
  include MethodChain

  module ClassMethods
    def performLoad(str)
      ConfigYAML.newFromYAML(str)
    end
  end

  def self.modified_loadmap(map)
    result = map.dup
    
    # remove variables that shouldn't be restored from the provided loadmap.
    config_vars_no_restore().each do |str|
      result.delete(str)
    end
    
    result
  end
    
  def config_version(query_class)
    if @config_yamlLoadMap
      validate_config_version(query_class)
      
      version_hash = @config_yamlLoadMap.fetch('config_version', {})
      
      if !version_hash.kind_of?(Hash)
        err { "#{query_class}: config version is not a hash, attempting fix (was #{version_hash.class})." }
        
        version_hash = if version_hash.kind_of?(Numeric)
          {YAMLClassReference.new(self.class) => version_hash}
        elsif version_hash.kind_of?(Array) 
          Hash[version_hash]
        else
          raise "Can't fix bad version hash."
        end 

        @config_yamlLoadMap['config_version'] = version_hash
      end
      
      version_hash.fetch(YAMLClassReference.new(self.class), 0)
    else
      raise "YAML load map is only available during configPostLoadTransform (#{self.class})."
    end
  end
    
  def config_yamlLoadMap=(val)
    @config_yamlLoadMap = val.dup
  end
  
  # hack? since this module is often included multiple times, the function is often overwritten.
  # possible fix is to modify config code to support mandate of ConfigYAML at base levels only?
  if !instance_methods.include?('to_yaml_type')
    def to_yaml_type
      "!david,2007/configyaml"
    end
  end
  
  # hack: since this module is often included multiple times, the function is often overwritten.
  # possible fix is to modify config code to support mandate of ConfigYAML at base levels only?
  if !instance_methods.include?('to_yaml')
    def to_yaml( opts = {}, is_cloning = false )
      YAML::quick_emit( nil, opts ) do |out|
        out.map( taguri, to_yaml_style ) do |map|
          outputYAML(map, is_cloning)
        end
      end
    end
  end
  
  def configyaml_clear_load_map
    begin
      remove_instance_variable(:@config_yamlLoadMap)
    rescue NameError
      raise "@config_yamlLoadMap was not defined for object of class '#{self.class}'"
    end
  end
  
  def configClone
    # this generally won't work for anything but the most basic objects
    # to get this done properly, the resolution step must proceed after id recalculation and then must
    # enact a mapping from the old object id to the new one.  
    newObj = ConfigYAML.newFromYAML(to_configstr)
    raise "Clone returned same object." if newObj.object_id == object_id()
    newObj.config_post_cloned
    newObj
  end

  def config_post_cloned
  end
  
  def _configPostLoadTransform(source, attributeReference, host)
    was_previously_transformed = !source.configNeedsTransform?
    
    result = super
    
    if !was_previously_transformed
      source.configyaml_clear_load_map
    end
    
    result 
  end
    
protected
  def config_hasEntryFor?(symbol)
    instance_variable_get("@#{symbol}") != nil
  end

  def configClassName
    self.class.name
  end

  def outputYAML(map, is_cloning = false)
    map.add('class', configClassName())
    
    version_hash = {}
    Algorithm.each_superclass(self.class) do |c|
      if c.respond_to?(:config_version)
        version_hash[YAMLClassReference.new(c)] = c.config_version if c.config_version > 0
      end
    end
    
    map.add('config_version', version_hash) if !version_hash.empty?
    
    if is_cloning
      map.add('was_cloned', true)
    end
    
    target = configTarget()
    
    if respond_to?(:configVariables)
      configVariables().each do |v|
        map.add(v.symbol.to_s, v.get(target)) if self.class.should_write_config_variable?(v.symbol.to_s, self, is_cloning)
      end
    end
  end
  
  def perform_save(stream)
    stream.write(to_yaml())
  end

  def self.newFromYAML(str)
    begin
      yaml = YAML.load(str)
      raise "No YAML object. String follows: #{str}" if !yaml
      
      yaml
    rescue Exception=>e
      puts "Couldn't construct YAML object (#{e})"
      puts e.backtrace
      raise DLB::Config::ExceptionLoad.new(e)
    end
  end
end


YAML::add_domain_type( "david,2007", "configyaml" ) do |type, val|
  begin
    objClass = eval(val['class'])
  rescue Exception=>e
    $stderr.puts "ConfigYAML couldn't deserialize a configyaml element's class. Error: #{e}"
    $stderr.puts "Contents of element:"
    $stderr.puts val.inspect
    nil
  end

  modified_loadmap = ConfigYAML.modified_loadmap(val)
  newObj = YAML.object_maker(objClass, modified_loadmap)
  newObj.config_on_created_during_load
  newObj.config_yamlLoadMap = val

  newObj
end

# hack: Viewable can insert instance variables on basic types (i.e. viewable_dependants.)
# Such modified strings convert to yaml including the instance variables. The code below fixes this.
class String
  alias_method :configyaml_to_yaml_fix_chain, :to_yaml
  def to_yaml(opts={})
    self[0..-1].configyaml_to_yaml_fix_chain(opts)
  end
end

class Hash
  def to_yaml(opts={})
    HashFix.new(self).to_yaml(opts)
  end
end

class HashFix
  yaml_as "tag:david,2009:HashFix"
  
  def initialize(hash)
    @hash = Array(hash)
  end
  
  def to_yaml(opts={})
    ary = [nil, nil]

    YAML::quick_emit( nil, opts ) do |out|
      out.seq( taguri, to_yaml_style() ) do |seq|
        @hash.each() do |k,v|
          seq.add([k, v])
        end
      end
    end
  end
  
  def self.yaml_new(klass, tag, val)
    hash = Hash.new
      
    val.each do |e|
      hash[e[0]] = e[1]
    end
    
    hash
  end
end

# catch yaml construction failures
class YAML::Object
  def configPostLoadTransform(source, attributeReference, host)
    $stderr.puts "ConfigYAML: Instance was not created properly by YAML: #{attributeReference.symbol} has class #{self.class}, does this class exist?"
    nil
  end
end

class YAMLClassReference
  include Raise
  
  yaml_as "tag:david,2009:class"
  
  attr_accessor :class_ref
  alias :to_class :class_ref
  
  def initialize(class_type)
    raise "class_type must be Class (supplied #{self.class})." if !class_type.kind_of?(Class)
    @class_ref = class_type
  end
  
  def eql?(other)
    case other
    when YAMLClassReference
      @class_ref.eql?(other.class_ref())
    when Class
      @class_ref.eql?(other)
    else
      super
    end
  end
  
  def hash
    @class_ref.hash
  end
  
  def self.yaml_new(klass, tag, val)
    self.new(eval(val['class']))
  end
  
  def to_yaml(opts={})
    YAML::quick_emit( nil, opts ) do |out|
      out.map( taguri, to_yaml_style ) do |map|
        map.add( 'class', @class_ref.name )
      end
    end
  end
end

end
