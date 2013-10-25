require 'std/ConfigYAML'
require 'std/module_logging'
require 'std/ObjectHost'
require 'std/SymbolDefaults'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'

class ActuatorHost
	include ConfigYAML
	include DRb::DRbUndumped
  include ModuleLogging
	include ObjectHost
	include SymbolDefaults
	include UniqueIDConfig
	include Viewable

	default_reader :hostedActuators, []
	attr_model_value :hostedActuators do 
    obj = ChangeActuator.new
    obj.actuatorHost = self
    [obj]
  end
  host :hostedActuators
  
	def hostActuator(a, instigator, sort=true, updateObjList=true)
		raise "Actuator already hosted" if a.actuatorHost && a.actuatorHost != self
    
    if updateObjList
		  @hostedActuators << a 
	    @hostedActuators.modified(ArrayModificationRecord.new(instigator, [@hostedActuators.length-1], 'add'))
    end
    
		a.actuatorHost = self
		
		# dbeswick: note: this call generally defeats the purpose of calling 'modified' with the
		# array modification event above. That's designed to help views do the minimum amount of
		# work in updating themselves, but this 'modified' call will generally cause the array's owning
		# view to regenerate. As a rule, only the object or variable directly affected should be
		# the subject of a 'modified' call. 
		#modified(self)
	end

	def unhostActuator(a, updateObjList=true, instigator=nil)
		raise "Target of function must respond to 'actuatorHost'" if !a.respond_to?(:actuatorHost)
  
    if !a.actuatorHost
      err { "Warning: attempt to remove an actuator with no 'actuatorHost' defined." }
    else
		  err { "Warning: attempt to remove actuator from non-owning host (host is '#{a.actuatorHost}')" } if a.actuatorHost != self
		end
  
		if updateObjList
		  remove_index = @hostedActuators.index(a) 
      @hostedActuators.delete_at(remove_index)

      if instigator
        @hostedActuators.modified(ArrayModificationRecord.new(instigator, [remove_index], 'remove'))
		  end
		end
		
		a.actuatorHost = nil
    modified(self)
	end

	def replace_actuator_with_class(actuator, actuator_class, instigator)
    newObj = actuator_class.createDefault
    current_id = actuator.unique_id 
    actuator.configCopyCommon(newObj)
    newObj.actuatorHost = nil
    
    actuator.uniqueid_unbind
    newObj.bind_id(current_id)
    
    replaceActuator(actuator, newObj, instigator)
    
    newObj
	end
	
	def replaceActuator(old, new, instigator=nil)
		raise "Attempt to replace actuator in non-owning host" if !self.hostedActuators.include?(old)
		
		new.actuatorHost.unhostActuator(new, true, nil) if new.actuatorHost
		
		new.actuatorHost = self
		
		oldIdx = @hostedActuators.index(old)
		
		# note: the old element is kept ([0..oldIdx]) so that it can be deleted in the following unhostActuator call.
		newArray = @hostedActuators[0..oldIdx] + [new] + @hostedActuators[oldIdx+1..-1]
		
    #tbd: this is done so that the viewable dependency links are maintained. Can I make that more robust so that replace is not required?
    @hostedActuators.replace(newArray)
        
		unhostActuator(old)
		
		if instigator
		  @hostedActuators.modified(ArrayModificationRecord.new(instigator, [oldIdx], 'replace'))
    end
      
		old.disconnect
	end
	protected :replaceActuator
	
	def updateActuators
		self.hostedActuators.each do |a|
			a.actuatorHost = self
			# some actuators may not yet be loaded, so check for resolution
			a.actuator_connect(a.sourceModifier) if (a.sourceModifier && a.sourceModifier.config_resolved?) || !a.needs_source_modifier?
		end
	end
	
	def actuatorModified
    modified(self)
	end
  
	def loaded
		super
		updateActuators
	end
end

