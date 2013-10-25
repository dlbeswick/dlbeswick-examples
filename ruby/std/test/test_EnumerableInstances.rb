require 'test/unit'
require 'date'
require 'yaml'
require 'EnumerableInstances'
require 'rubygems'
require 'ruby-debug'

class TC_EnumerableInstances < Test::Unit::TestCase
  class Base
  	include EnumerableInstances

		def test_derived_instances
			instances()
		end
  end
  
  class Derived < Base
  end

	class X
		include EnumerableInstances
	end

	module M
		include EnumerableInstances
		
		def bye
			remove_instance
		end
	end
	
	class CM
		include M
  end

  class ::Date
    include EnumerableInstances
  end
  
  class MultipleInclusionBase
    include EnumerableInstances
  end

  class MultipleInclusion < MultipleInclusionBase
    include EnumerableInstances
  end
  
  module ExtendMe
    include EnumerableInstances
  end
  
  def test_date_yaml
    d = Date.today
    assert(YAML.load(d.to_yaml) == d, "Date is not restoring from YAML correctly after including EnumerableInstances.")
  end

  def test_module_instances
    cm = CM.new

    assert(
      !M.instances.empty?,
      "No instances were stored in module 'M' of a class including that module."
    )

    assert(
      M.instances.length == 1,
      "'#{ExtendMe.instances.length}' instances were stored in module 'M' of a class including that module."
    )
  end

  def test_derived_inclusion
    i = MultipleInclusion.new

    assert(
      MultipleInclusion.instances.length == 1,
      "'#{MultipleInclusion.instances.length}' instances were stored in a derived class including EnumerableInstances both in itself and in the base class."
    )

    j = MultipleInclusionBase.new
    
    assert(
      MultipleInclusionBase.instances.length == 2,
      "'#{MultipleInclusionBase.instances.length}' instances of MultipleInclusionBase, expected 2 instances."
    )
  end
  
  class LateBaseA
  end

  class LateBaseB < LateBaseA
    include EnumerableInstances
  end
  
  class LateBaseA
    include EnumerableInstances
  end
  
  def test_late_base_multiple_inclusion
    i = LateBaseB.new
    
    assert(
      LateBaseB.instances.length == 1,
      "'#{LateBaseB.instances.length}' instances of LateBaseB, expected 1 instances."
    )
  end
  
	def test_extend_included
		s = String.new
		
		s.extend(ExtendMe)
		
		assert(
			s.respond_to?(:weakrefInstances) && s.weakrefInstances, 
			"A module extending 'EnumerableInstances' did not inherit the required methods."
		)
		
		assert(
			!ExtendMe.instances.empty?,
			"No instances were stored of a module extending 'EnumerableInstances'."
		)
		
		assert(
			!s.instances.empty?,
			"No instances were stored of a module extending 'EnumerableInstances'."
		)
		
		assert(
			ExtendMe.instances.length == 1,
			"'#{ExtendMe.instances.length}' instances were stored of a module extending 'EnumerableInstances'."
		)
		
		assert_nothing_raised("'remove_instance' from module method extending 'EnumerableInstances' failed.") do
			s.remove_instance
		end

		assert(
			s.instances.empty?,
			"Instances was not removed when module extending 'EnumerableInstances'."
		)
	end
		
	def test_module_inherit
		cm = CM.new

		assert(
			cm.respond_to?(:weakrefInstances) && cm.weakrefInstances, 
			"A class derived from a module including 'EnumerableInstances' did not inherit the required methods."
		)
		
		assert(
			!cm.instances.empty?,
			"No instances were stored of a class derived from a module including 'EnumerableInstances'."
		)
		
    assert(
      cm.instances.length == 1,
      "'#{cm.instances.length}' instances were stored of a class derived from a module including 'EnumerableInstances'."
    )
    
		assert_nothing_raised("'remove_instance' from module method including 'EnumerableInstances' failed.") do
			cm.bye
		end

		assert(
			cm.instances.empty?,
			"Instances was not removed when class derived from a module including 'EnumerableInstances'."
		)
	end
	
	def test_remove
		x = X.new
		
		assert_nothing_raised("'remove_instance' failed.") do
			x.remove_instance
		end
		
		assert_block("'remove_instance' did nothing.") do
			x.instances.empty?
		end
	end
	
	def test_finalize
		x = X.new
		xx = X.new
		
		assert(!X.instances.empty?, "No instances exist.")
		
		x = nil
		xx = nil
		
		assert_nothing_raised("Test objects were garbage-collected prematurely.") do
			X.instances
		end

		GC.enable
		GC.start
		
		assert_nothing_raised("Garbage-collected objects were not removed from the instances array.") do
			X.instances
		end

		assert(X.instances.empty?, "Instances exist.")
	end
	
	def test_derived_instances
		a = Base.new
		d = Derived.new
		
		assert(!a.instances.empty?, "No class instances returned from base <instance>.instances")
		
		assert(!d.test_derived_instances.empty?, "No class instances returned from derived <instance>.instances") 
	end
end
