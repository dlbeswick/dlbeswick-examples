require 'std/ConfigYAML'
require 'std/DefaultCreateable'
require 'std/EnumerableSubclasses'
require 'std/IncludableClassMethods'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'

# An Actuator is an abstraction through which the user manipulates SourceModifiers.
# In the case that many Actuators are mapped to a single SourceModifier, only one Actuator may
# affect the SourceModifier per update, and all other Actuators must update their internal state
# to correspond with the new state of the SourceModifier.
#
# Actuators must call onModifiedSource when they affect their target SourceModifier, and will receive
# updateFromSource when they must update their state to reflect the SourceModifier.
module Actuator
	include ConfigYAML
	include DefaultCreateableOwned
	include EnumerableSubclasses
	include IncludableClassMethods
	include UniqueIDConfig
	include Viewable

	attr_accessor :actuatorHost
	attr_infer :name, ''
	attr_reader :sourceModifier
  
  no_view :sourceModifier
  no_view :actuatorHost
  no_view :hostedObjects
  no_view :name
    	
	persistent :name
	persistent :sourceModifier
	
	def initActuator(actuatorHost)
    extend(Mutex_m)
		@actuatorHost = actuatorHost
		@actuatorHost.hostActuator(self, self) if @actuatorHost
		self.ensure_id
	end

	module ClassMethods
		def createDefault
			DRb.send_authoritive(false, self, :authoritive_create_default)
		end
		
		def createDefaultWithOwner(owner)
      DRb.send_authoritive(false, self, :authoritive_create_default_with_owner, owner)
		end

		def new_from_default
		  self.new()
		end
		
		def visual?
			false
		end
	end
	
  def needs_source_modifier?
    true
  end

	def actuator_connect(sourceModifier)
    if !sourceModifier && needs_source_modifier?
      $warn.info "Attempt to connect actuator with ID '#{self.uniqueID}' to nil"
      $warn.info "Host ID '#{@actuatorHost.uniqueID}'" if @actuatorHost
      return
    end
    
 		@sourceModifier = sourceModifier
	  
    @sourceModifier.onActuatorConnected(self) if @sourceModifier
    updateFromSource(sourceValue())
	end

	def ordering(rhs)
		0
	end
	
	def newFromSelf
		newObj = configClone
		
		if self.name
			newObj.name = "new " + self.name
		end
		
		newObj.actuatorHost = nil
		
		newObj
	end
	
	def viewable_name
		return @name if @name
		super
	end
	
	def viewable_description
		return "#{viewable_name()} hosted by #{@actuatorHost.viewable_name}" if @actuatorHost
		super
	end
	
	def disconnect
		@sourceModifier.onActuatorDisconnected(self) if @sourceModifier
	end

	def updateFromSource(value)
	end
	
	def sourceValue=(value)
		@sourceModifier.set_value(value, self)
		onModifiedSource
	end
	
	def sourceValue
    if @sourceModifier.nil?
      nil
    else
      @sourceModifier.value
    end
	end
	
	def reset
	end

	def source
		return @sourceModifier.sources[0]
	end

	def onModifiedSource
		# temp -- for midi learn testing
		#@sourceModifier.actuators.each do |a|
		#	a.sendLearnData if a.kind_of?(MIDIActuator::Base)
		#end
	end

	def update_on_source_modify?
	  true
	end
end

