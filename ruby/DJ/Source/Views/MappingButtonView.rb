require './Source/Views/MappingEditor'
require 'std/PersistentWindow'

class MappingButtonViewBase < ViewBasicAttributeReference
  include PlatformGUISpecific

  suits Mapping::Base

protected

  def make_controls
    makeEditButton
  end
  
  def onEditClicked
    ViewContainers::Single.new(attribute(), self, MappingEditor)
  end
end

class MappingButtonViewFox < MappingButtonViewBase
  include PlatformGUISpecific
  
protected
  def make_controls
    @horz = FXHorizontalFrame.new(self)
    @horz.create
    super
  end

  def makeEditButton
    FXButton.new(@horz, 'E') do |ctrl|
      ctrl.create
  
      ctrl.connect(SEL_COMMAND) do
        onEditClicked
      end
    end
  end
  
  def graphViewParent
    @horz
  end
end

class MappingButtonViewWx < MappingButtonViewBase
  include PlatformGUISpecific
	
	def generate
		super
		renderBitmap
	end

protected
  def renderBitmap
  	@bitmap.draw do |dc|
  		@renderer.doPaint(dc, self.target())
  	end
  	@bitmapButton.bitmap_label = @bitmap
  end
  
  def makeEditButton
  	@renderer = GraphRendererMapping.new
  	@renderer.gridDivisions = 0
  	@renderer.num_samples = 16
  	
  	@bitmap = Wx::Bitmap.new(16, 16)
  
    @bitmapButton = Wx::BitmapButton.new(self, -1, @bitmap) do |ctrl|
      ctrl.parent.evt_button(ctrl) do
        onEditClicked
      end
      
      self.sizer.add(ctrl)
    end
  end
end
