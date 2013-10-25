require 'test/unit'
require 'IncludableClassMethods'

class TC_IncludableClassMethods < Test::Unit::TestCase
  class NoIncludeTest
  end

  module BaseModule
    include IncludableClassMethods
    
    module ClassMethods
      def classMethod
        true
      end
    end
  end
  
  module BaseModuleAlternate
    include IncludableClassMethods
    
    module ClassMethods
      def classMethodAlternate
        true
      end
    end
  end
  
  class BaseClass
    include BaseModule
  end

  module IncludeTestModule
    include BaseModule
    include IncludableClassMethods
    
    module ClassMethods
      def classMethod2
        true
      end
    end
  end
  
  module IncludeTestModuleClientModule
    include IncludeTestModule
    include BaseModuleAlternate
    
    module ClassMethods
      def classMethod3
        true
      end
    end
  end
  
  class IncludeTestClass
    include IncludeTestModule
  end
  
  class IncludeTestModuleClientClass
    include IncludeTestModuleClientModule
  end
  
  def test_noinclude
    assert_raise(RuntimeError) do
      NoIncludeTest.module_eval('include IncludableClassMethods')
    end
  end
  
  def test_base_include
    assert(
      BaseClass.methods.include?('classMethod'),
      'A class that included the base module did not finally include its class methods.'
    )
    
    assert_send([BaseClass, :classMethod], 'Bad result from class method.')
  end
  
  def test_module_include
    assert_send([IncludeTestModule, :classMethod], 'A module that included the base module did not finally include the base module\'s class methods.')
    
    assert_send([IncludeTestModuleClientModule, :classMethod], 'Bad result from class method in client module.')
    assert_send([IncludeTestModuleClientModule, :classMethod2], 'Bad result from class method in client module.')
    
    assert_send([IncludeTestClass, :classMethod],
      'A class that included a module at depth 2 did not finally include the base module\'s class methods.'
    )
    assert_send([IncludeTestClass, :classMethod2],
      'A class that included a module at depth 2 did not finally include the base module\'s class methods.'
    )

    assert_send([IncludeTestModuleClientClass, :classMethod],
      'A class that included a module at depth 3 did not finally include the base module\'s class methods.'
    )
    assert_send([IncludeTestModuleClientClass, :classMethod2],
      'A class that included a module at depth 3 did not finally include the base module\'s class methods.'
    )
    assert_send([IncludeTestModuleClientClass, :classMethod2],
      'A class that included a module at depth 3 did not finally include the base module\'s class methods.'
    )
  end
  
  def test_module_multiple_sources
    assert_send([IncludeTestModuleClientModule, :classMethodAlternate], 'A module that included another client of IncludableClassMethods did not propagate its class methods.')
  end
  
  puts "Recursion test, wait:"
  @@recurse_status = begin
    module Recurse
      include IncludableClassMethods
      
      module ClassMethods
      end
    end
    
    module Recurse2
      include Recurse
      include IncludableClassMethods
      
      module ClassMethods
      end
    end

    module Recurse3
      include Recurse2
    end
      
    puts "Recursion test ok."
    
    true
  rescue Exception=>e
    puts "\n\nRecursion test failed."
    p e
    false
  end
  
  def test_recurse
    assert( @@recurse_status == true, "Recursively including IncludableClassMethods failed." )
  end
end