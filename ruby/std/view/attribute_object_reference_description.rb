require 'std/view/labeled_field'
require 'std/view/ViewContainer'

module Views
  
  # Displays an object instance. Shows the result of viewable_description on the instance and opens a window
  # showing a view of the object when an accompanying 'Open' button is clicked.
  class AttributeObjectReferenceDescription < ObjectReference
    include AttributeReferenceView
    
    def self.order
      -5
    end
    
    def self.suits_attribute?(attribute)
      attribute.viewable_kind_of?(AttributeReference::Base)
    end
    
    def self.suits_target?(target, target_class)
      true
    end
    
  protected
    def make_controls
      description = if target().nil?
        'nil'
      else
        attribute().viewable_summary
      end
      
      layoutAddHorizontal(makeStaticText(self, description))

      if !Views.basic_type?(target())
        button = makeButton(self, 'Open') do
          ViewContainers::Single.new(AttributeReference::ObjectReadOnly.new(target()), self)
        end
        
        layoutAddHorizontal(button, 5)
      end
    end
  end
  
end

  
