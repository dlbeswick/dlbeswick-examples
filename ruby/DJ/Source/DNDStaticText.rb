require './Source/DNDArea'

module DNDStaticTextCommon
protected
	def init_dndstatictextcommon(dndInjectTarget, dnd, dndData)
		DNDArea.inject(dndInjectTarget, dnd, dndData)
	end
end

class DNDStaticTextWx < Wx::Panel
	include PlatformGUISpecific
	include DNDStaticTextCommon
	
	attr_accessor :text

	def initialize(parent, text, dnd, dndData=nil)
  	super(parent)

  	init_dndstatictextcommon(self, dnd, dndData)
  	
 		enable
  	
    text = Wx::StaticText.new(self, -1, text)
    
    self.background_colour = Wx::WHITE
    self.own_background_colour = Wx::WHITE
    self.fit
    
    max = Math.max(size.x, size.y) + 5
    
    self.initial_size = Wx::Size.new(max, max)
    self.min_size = Wx::Size.new(max, max)
    
    text.center(Wx::BOTH)
  end
end

