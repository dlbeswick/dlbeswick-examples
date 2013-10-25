module Abstract
  def self.included(receiver)
    # This variable is also used by EnumerableSubclasses
    # TBD: remove 'abstract' data from EnumerableSubclasses? 
    receiver.instance_variable_set(:@abstract, true)
    receiver.instance_eval do
      def abstract_class_method
        raise "This method is abstract. Implementation should be supplied by subclasses."
      end
    end
  end
  
  def abstract?
    true
  end
  
  def abstract
    raise "This method is abstract. Implementation should be supplied by subclasses."
  end
end