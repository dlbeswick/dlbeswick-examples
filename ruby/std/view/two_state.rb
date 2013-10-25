require 'std/abstract'
require 'std/view/View'

class ViewTwoState < ViewBasicAttributeReference
  include Abstract
  
  def make_controls
    control = makeCheck(self, '', target()) do |value|
      value = if value
        self.class.possible_values[0]
      else
        self.class.possible_values[1]
      end
      
      attribute().set(value)
      attribute().modified(self)
    end
    
    set_control_enabled(control, attribute.writable?)
    
    layoutAddVertical control
  end
  
  def self.order
    -1
  end
  
  def self.possible_values
    abstract
  end
  
  def self.suits_target?(target, target_class)
    possible_values.include?(target)
  end
end

class ViewTwoStateBool < ViewTwoState
  def self.possible_values
    [true, false]
  end
end
