require 'test/unit'
require 'EnumerableSubclasses'

class TC_EnumerableSubclasses < Test::Unit::TestCase
  class BaseClass
    include EnumerableSubclasses
  end
  
  class DerivedClass < BaseClass
  end
  
  class DerivedAbstractClass < BaseClass
    abstract
  end
  
  module BaseModule
    include EnumerableSubclasses
  end
  
  class ClassIncludingBaseModule
    include BaseModule
  end
  
  module BaseModuleClientModule
    include BaseModule
    include EnumerableSubclasses
  end
  
  class ClassIncludingBaseModuleClient
    include BaseModuleClientModule
  end
  
  class DerivedClassIncludingBaseModuleClient < ClassIncludingBaseModuleClient
  end
  
  class MultiBaseModuleIndirectIncludeClass
    include BaseModule
    include BaseModuleClientModule
  end 
  
  def test_subclasses
    assert( !BaseClass.subclasses.empty?, 'No BaseClass-derived classes were found.' )
    assert( !BaseClass.subclasses.include?(BaseClass), 'BaseClass includes itself in subclasses.' )
    assert( BaseClass.subclasses.uniq == BaseClass.subclasses, 'Duplicate subclasses in list.' )
  end
  
  def test_subclasses_abstract
    assert( !BaseClass.subclasses.include?(DerivedAbstractClass), 'An abstract BaseClass-derived class was included in the subclasses list.' )
  end
  
  def test_module_direct
    subclasses = BaseModule.subclasses
    assert( subclasses.uniq == subclasses, 'Duplicate BaseModule subclasses in list.' )
    
    assert( !subclasses.empty?, "No subclasses were listed for 'BaseModule'" )
    assert( subclasses.include?(ClassIncludingBaseModule), "'ClassIncludingBaseModule' was not listed as a subclass of 'BaseModule.'" )
    assert( subclasses.include?(ClassIncludingBaseModuleClient), "'ClassIncludingBaseModuleClient' was not listed as a subclass of 'BaseModule.'" )
  end

  def test_module_indirect
    subclasses = BaseModule.subclasses
    
    assert( subclasses.include?(DerivedClassIncludingBaseModuleClient), "'DerivedClassIncludingBaseModuleClient' was not listed as a subclass of 'BaseModule.'" )
  end
  
  def test_client_module_has_subclasses
    assert( !BaseModuleClientModule.subclasses.empty?, "Client module of an E.S.C. module included E.S.C. itself, but does not have subclasses listed." )
    assert( BaseModuleClientModule.subclasses.uniq == BaseModuleClientModule.subclasses, 'Duplicate subclasses in list.' )
  end
end
