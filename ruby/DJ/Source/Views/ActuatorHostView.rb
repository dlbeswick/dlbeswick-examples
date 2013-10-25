require 'std/view/View'
require 'std/profile_help'

class ActuatorHostViewBase < ViewBasicAttributeReference
  include PlatformGUISpecific
  
  suits ActuatorHost
  
  def initialized
  	@views = []
  end
  
	def self.order
		-1
	end
	
protected
	def addActuatorViews(parent)
    # tbd: not good to modify the hosted objects array in a view function.
    # should add a 'sort' ability to the array view, which will mandate
    # an assumption that the view of hostedObjects is an array view.
	  # xdrb 
 		#target().hostedObjects.sort! do |lhs, rhs|
    #  if lhs.respond_to?(:ordering)
    #    lhs.ordering(rhs)
    # else
    #    1
    #  end
    #end

	  Progress.run("Viewing actuator host '#{self.viewable_name}'") do
	    view = View.viewAttribute(:hostedActuators, target(), parent)
	    layoutAddVertical(view)
	    @views << view
	  end
	end
end

class ActuatorHostViewWx < ActuatorHostViewBase
  include PlatformGUISpecific

	def make_controls
    Wx::Button.new(self, -1, "Save") do |b|
      self.sizer.add(b)

      self.evt_button(b) do
        target().save
      end
    end
	end

  def generate
  	while !@views.empty?
  		@views.shift.destroy
  	end
  	
    addActuatorViews(self)
  end
end

class ActuatorHostViewFox < ActuatorHostViewBase
  include PlatformGUISpecific
  
  def generate
    clear

    FXHorizontalFrame.new(self) do |pack|
      pack.create
      
      FXButton.new(pack, 'Save') { |b|
        b.create
        b.connect(SEL_COMMAND) {
          target().save
        }
      }
    end

    if !@scroll
      @scroll = FXScrollWindow.new(parent) do |scroll|
        scroll.layoutHints = LAYOUT_FILL
        scroll.create
        @scrollPacker = FXPacker.new(scroll, FRAME_RAISED) do |pack|
          pack.padLeft = 0
          pack.padRight = 0
          pack.padTop = 0
          pack.padBottom = 0
          pack.vSpacing = 0
          pack.layoutHints = LAYOUT_FILL
          pack.create
        end
      end
    end
    
    addActuatorViews(@scrollPacker)
  end
end
