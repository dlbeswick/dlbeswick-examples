require 'test/unit'
require 'std/view/Viewable'

class A_Viewable
  include Viewable
end

class B_Observer
  def onViewableDependencyEvent
    puts "Viewable dependency event."
  end
  
  def hello
    puts 'Hello'
  end
end

class TC_Viewable < Test::Unit::TestCase
  def b_observer_count
    count = 0
    
    ObjectSpace.each_object do |obj|
      count += 1 if obj.kind_of?(B_Observer)
    end
    
    count
  end
  
  def b_finalizer_proc
    proc { puts 'B finalized.' }
  end
  
  def test_dependencies_0
    @a = A_Viewable.new
    b = B_Observer.new
    @b_id = b.object_id
    
    @a.viewable_dependant(b)
    
    assert(@a.viewable_dependants.length == 1, "Dependant appears to have not been added.")
    assert(Viewable.dependant_to_masters(@b_id).length == 1, "Dependant-to-masters entry appears to have not been added.")
    
    ObjectSpace.define_finalizer(b, b_finalizer_proc)
    assert(b_observer_count() == 1, "Observer should be alive.")
  end
  
  def test_dependencies_1
    puts 'GC start.'
    GC.start
    
    puts 'GC done.'
    assert(b_observer_count() == 0, "Observer not GC'ed")
    
    assert(@a.viewable_dependants.length == 0, "Observer not removed from dependants list.")
    assert(Viewable.dependant_to_masters(@b_id).length == 0, "Dependant-to-masters entry appears to have not been removed.")
  end
end
