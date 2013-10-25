require './Source/Views/ActuatorView'
require './Source/DataSelector'
require './Source/DNDArea'
require './Source/DNDDataActuatorLink'
require './Source/midiactuator'

class MIDIActuatorView < ActuatorView
  suits MIDIActuator::Base
  
	def <=>(rhs)
		super if !self.target || !rhs.target
		self.target.get.listenerCC.message.controlCode <=> rhs.target.get.listenerCC.message.controlCode
	end
	
  def makeAttributesView
    # tbd: add a "add to horizontal layout" function that accepts a view, for cross-guiplatform
    # tbd: layoutAddHorizontal fills this role now, figure out how to use it.
    @horz.add(View.viewAttribute(:mapping, target(), self))
    @horz.add(View.viewAttribute(:controlCode, target().listenerCC.message, self))
  end
	
	def self.order
		10
	end
end

