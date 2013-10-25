require './Source/ChangeActuator'
require './Source/Views/ActuatorView'

class ChangeActuatorView < ActuatorView
  suits ChangeActuator
  
	def make_controls
		super
		
		actuator = target()
		
  	text = DNDStaticText.new(self, "Target", $djgui.dragAndDrop)
		text.dndarea_accept?(DNDData::Object) do |data|
			data.object.viewable_kind_of?(SourceModifier) && data.object != actuator.sourceModifier
		end
		
    text.dndarea_accept(DNDData::Object) do |data|
			actuator.addTarget(data.object)
      actuator.modified(self)
		end

		layoutAddHorizontal(text)
	end

	def self.order
		1
	end
end
