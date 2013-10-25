require 'std/view/View'

class GUIMessage
  include Viewable
  
  attr_reader :message
  attr_reader :title
  
  def initialize(message, title='')
    @message = message
    @title = title
  end
end

class ViewMessageBox < ViewBasicAttributeReference
  suits(GUIMessage)
  
  def generate
    view_destroy_children
    layoutAddVertical makeStaticText(self, target().message)
    layoutAddVertical makeButton(self, 'Ok') { container().destroy(); }#GC.start }
  end
  
  def viewable_name
    target().title
  end
  
  def self.order
    -0.9
  end
end
