require 'test/unit'
require 'ConfigYAML'
require 'ObjectHost'
require 'UniqueIDConfig'

class TC_ConfigYAML < Test::Unit::TestCase
  class UniqueIDType
    include UniqueIDConfig
    
    persistent :text
    
    def initialize
      @text = "I'm hosted."
    end
  end
  
  class Test
    include ConfigYAML
    include ObjectHost
    
    persistent :int
    persistent :string
    persistent :array
    persistent :array_uniqueid
    
    host :array_uniqueid
    
    def initialize
      @int = 1
      @string = 'hi'
      @array = [1,2,3,'hi']
      @array_uniqueid = [UniqueIDType.new, UniqueIDType.new]
    end
  end
  
  @@test_output = <<EOS 
--- !david,2007/configyaml 
class: TC_ConfigYAML::Test
config_version: 0
hostedObjects: !seq:ObjectHost::HostedObjects 
  - !david,2007/configyaml_uniqueid_content 
    unique_id: 0
    class: TC_ConfigYAML::UniqueIDType
    config_version: 0
    text: I'm hosted.
  - !david,2007/configyaml_uniqueid_content 
    unique_id: 1
    class: TC_ConfigYAML::UniqueIDType
    config_version: 0
    text: I'm hosted.
int: 1
string: hi
array: 
- 1
- 2
- 3
- hi
array_uniqueid: 
- !david,2007/uniqueid 
  unique_id: 0
- !david,2007/uniqueid 
  unique_id: 1
EOS
  
  def test
    instance = Test.new
    
    output = instance.to_configstr
    assert(output == @@test_output, "#{@@test_output}\n\n!=\n\n#{output}")
  end

  def test_loadmap_removal
    instance = Test.new.load(StringIO.new(Test.new.to_configstr))
    assert(!instance.instance_variable_defined?(:@config_yamlLoadmap), "Loaded instance had @config_version defined.")
  end
  
  def test_version_removal
    instance = Test.new.load(StringIO.new(Test.new.to_configstr))
    assert(!instance.instance_variable_defined?(:@config_version), "Loaded instance had @config_version defined.")
  end
end