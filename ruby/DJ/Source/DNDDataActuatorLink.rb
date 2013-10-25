require './Source/DNDData'

module DNDData

  class SourceModifierReference < Base
  	attr_accessor :sourceModifier
  
  	def initialize(sourceModifier)
  		@sourceModifier = sourceModifier
  	end
  
  	def draggable?
  		false
  	end
  end
  
  class ActuatorReference < Base
  	attr_accessor :actuator
  
  	def initialize(actuator)
  		@actuator = actuator
  	end
  
  	def draggable?
  		true
  	end
  end
  
  class ActuatorLinkToSourceModifier < ActuatorReference
  	attr_accessor :actuator
  
  	def initialize(actuator)
  		@actuator = actuator
  	end
  
  	def dropped(target)
  		@actuator.disconnect
  	end
  
  	def description
  		if @actuator.sourceModifier
  			"Linked to #{@actuator.sourceModifier.viewable_name} #{@actuator.sourceModifier.viewable_description}"
  		else
  			'Not linked.'
  		end
  	end
  end
  
  class ActuatorActionTarget < ActuatorReference
  end
end
