require 'std/view/View'
require 'std/abstract' 

class ViewLabeledField < ViewBasicAttributeReference
  include Abstract
  
  def make_controls
    name = attribute().viewable_name
    
    if !name.empty?
      layoutAddHorizontal(makeStaticText(self, name))
      makeField(15)
    else
      makeField(0)
    end
  end

protected
  def makeField(padding)
    abstract
  end
end
