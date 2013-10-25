require 'std/view/View'

class ViewBorderedWx < ViewBasicAttributeReference
  include Abstract
  include PlatformGUISpecific
  
  attr_writer :border

  def border
    @border ||= true
  end  
  
  def makeSizer(target)
  end
  
  def make_controls
    return if target().viewableAttributes().empty?

    if border()
      self.sizer ||= Wx::StaticBoxSizer.new(Wx::VERTICAL, self, border_text())
    else
      self.sizer ||= Wx::BoxSizer.new(Wx::VERTICAL)
    end
    
    super
  end
  
protected
  def border_text
    if !attribute().viewable_description.empty?
      attribute().viewable_description
    else
      attribute().viewable_name
    end
  end
end
