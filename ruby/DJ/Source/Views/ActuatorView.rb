require './Source/DNDStaticText'
require 'std/view/View'
require './Source/Views/MappingEditor'

class ActuatorViewBase < ViewBasicAttributeReference
  include PlatformGUISpecific

  suits Actuator
  
  def initialize(target, parentWindow)
		super
		
		evt_child_focus do |evt|
		  # stop scrollwindow jittering on focus change
		end
	end
	
	def hasMapping?
		target().respond_to?(:mapping)
	end
	
	def mapping
		return nil if !hasMapping?
		target().mapping
	end
	
protected
	def layoutAddHorizontal(who)
		PlatformGUI.specific
	end
	
  def nameFieldText
    target().viewableName
  end
  
  def make_controls
    makeLinkArea
    makeCopyButton
    makeNameField
    makeAttributesView
  end
  
  def attributesViewParent
    self
  end
  
  def makeAttributesView
    layoutAddVertical(ViewAllAttributesReference.new(AttributeReference::ObjectReadOnly.new(target()), attributesViewParent(), false))
  end
  
  def makeLinkArea
  	text = DNDStaticText.new(self, "Link", $djgui.dragAndDrop, DNDData::Object.new(target()))

   	layoutAddHorizontal(text)
  end
  
  def makeCopyButton
    PlatformGUI.specific
  end

  def makeNameField
    view = View.viewAttribute(:name, target(), self, nil, Views::AttributeReferenceTextFieldBasicType)
    view.onRightClick = proc do
      onNameFieldRightClick()
    end
    
    layoutAddHorizontal(view)
  end

  def onCopyButton
    actuator = target()
    actuator.actuatorHost.hostActuator(actuator.newFromSelf, self)
  end
  
  def onNameFieldRightClick
    changeClass
  end
  
  def changeClass
    selections = GUISelections.new(
      "Select a new class for the actuator '#{target().viewable_name()}'",
      Actuator.subclasses,
      target().viewable_class,
      "Actuator class change"
    ) do |selected_class|
      host = target().actuatorHost
      replacement = host.replace_actuator_with_class(target(), selected_class, self)
      # replacing the object that the attribute references modifies the attribute
      attribute().modified(self)
    end
    
    ViewContainers::Single.new(selections, self)
  end
end

class ActuatorViewFox < ActuatorViewBase
  include PlatformGUISpecific

protected
  def attributesViewParent
    FXPacker.new(self)
  end
  
  def make_controls
    @packer = FXHorizontalFrame.new(self)
    @packer.create
    
    super
  end
  
  def makeCopyButton
    FXButton.new(@packer, 'cp') do |ctrl|
      ctrl.connect(SEL_COMMAND) do
        onCopyButton
      end
      ctrl.create
    end
  end
  
  def makeNameField
    FXTextField.new(@packer, 10) { |ctrl|
      ctrl.text = nameFieldText
      
      ctrl.connect(SEL_COMMAND) do
        onNameFieldEnter
        ctrl.selectAll
      end
      
      ctrl.connect(SEL_LEFTBUTTONPRESS) do
        ctrl.selectAll
      end
      
      ctrl.connect(SEL_RIGHTBUTTONPRESS) do
        onNameFieldRightClick
      end
      
      ctrl.enable
      ctrl.create
    }
  end
end

class ActuatorViewWx < ActuatorViewBase
  include PlatformGUISpecific

protected
	def layoutAddHorizontal(who)
		@horz.add(who)
	end
	
  def make_controls
    @horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
    self.sizer.add(@horz)
    super
  end
  
  def makeCopyButton
    Wx::Button.new(self, -1, 'cp', :style => Wx::BU_EXACTFIT) do |ctrl|
      ctrl.parent.evt_button(ctrl) do
        onCopyButton
      end
      
      @horz.add(ctrl)
    end
  end
end
