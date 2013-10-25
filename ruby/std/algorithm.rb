module Algorithm
  def self.alias_class_method(module_obj, symbol_alias, symbol_old)
    Algorithm.singleton_eval(module_obj, "alias_method :#{symbol_alias}, :#{symbol_old}")
  end

  # Passes the result of object.<method_symbol> to the block.
  # The first call to the block is the result of the object call, and not 
  # object.
  def self.recurse(object, method_symbol, *args, &block)
    while object do
      object = object.send(method_symbol, *args)

      yield object if object
    end
  end

  # As for 'recurse', but 'object' is passed to the first invocation of 'block',
  # then the results of the method calls as usual.
  def self.recurse_from_object(object, method_symbol, *args, &block)
    while object do
      yield object
      object = object.send(method_symbol, *args)
    end
  end

  def self.each_superclass(class_object, &block)
    while class_object do
      yield class_object if class_object
      class_object = class_object.superclass
    end
  end

  # Yields the hierarchy of the object's ancestors to the block.
  # Accepts a class, module or object.
  # In the case of an object, ancestors begin with the object's singleton class.
  def self.each_ancestor(classOrModuleOrObj, includeSelf = false, &block)
    if classOrModuleOrObj.kind_of?(Module)
      classOrModuleOrObj.ancestors.each do |a|
        yield a if includeSelf || a != classOrModuleOrObj
      end
    else
      each_ancestor_singleton(classOrModuleOrObj, &block)
    end
  end

  # Yields the hierarchy of the object's ancestors to the block, if the ancestor inherits from or includes 'moduleType'.
  # Accepts a class, module or object.
  # In the case of an object, ancestors begin with the object's singleton class.
  # The exact class given by 'moduleType' is only passed to the block if 'includeBase' is true.
  def self.each_ancestor_kindof(moduleOrObj, moduleType, includeSelf = false, includeBase = false, &block)
    each_ancestor(moduleOrObj, includeSelf) do |a|
      if includeBase
        yield a if a <= moduleType
      else
        yield a if a < moduleType
      end
    end
  end
  
  def self.singleton_class(object)
    class << object
      self
    end
  end
  
  # Perform a 'module_eval' in the singleton class.
  # Tha action of defining methods here is equivalent to calling 'def self.method' in a standard class definition.
  # In other words, defining methods in the 'singleton' context of a class is equivalent to defining class methods
  # on that class. 
  def self.singleton_eval(object, string)
    singleton_class(object).module_eval(string)
  end
  
  # Performs each_ancestor beginning at the given object's singleton class.
  # It doesn't make sense to preclude the subject's class in this case (as opposed to each_ancestor,)
  # because that would be the equivalent operation to "each_ancestor(object.class, false)"
  def self.each_ancestor_singleton(object, &block)
    singleton_class(object).ancestors.each do |a|
      yield a
    end
  end
  
  # Return the first item in a sorted array that is equal to or greater than 'item'.
  # tbd: actually looks like upper bound, oops
  def self.lower_bound(array, item, &test)
    return 0 if array.empty?

    lower_bound = nil
    upper_bound = nil
    new_lower_bound = -1
    new_upper_bound = array.size
    
    begin
      lower_bound = new_lower_bound
      upper_bound = new_upper_bound
      
      test_idx = (lower_bound + (upper_bound - lower_bound) / 2)
      break if test_idx == array.size || test_idx == -1

      if test
        if yield(item, array[test_idx]) < 0
      	  new_upper_bound = test_idx
        else
      	  new_lower_bound = test_idx
        end
      else
        if (item <=> array[test_idx]) < 0
          new_upper_bound = test_idx
        else
          new_lower_bound = test_idx
        end
      end
      
      #			puts "#{new_lower_bound} ... #{upper_bound}"
      #			sleep(0.5)
    end until new_lower_bound == lower_bound && new_upper_bound == upper_bound
    
    lower_bound + 1
  end

  def self.test_lower_bound(array, item)
    idx = self.lower_bound(array, item) do |lhs, rhs|
      puts "#{lhs} <=> #{rhs} == #{lhs <=> rhs}"
      lhs <=> rhs
    end
    p array.insert(idx, item)
  end
  
  #	a = []
  #
  #	p a
  #	test_lower_bound(a, 1)
  #	test_lower_bound(a, 5)
  #	test_lower_bound(a, 3)
  #	test_lower_bound(a, 4)
  #	test_lower_bound(a, 2)
  #	test_lower_bound(a, 10)
  #	test_lower_bound(a, 0)
  #	
  #	debugger
end
