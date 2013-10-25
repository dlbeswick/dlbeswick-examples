require './Source/actuator'
require 'std/ObjectHost'
require 'std/SymbolDefaults'
require 'drb'
require 'std/view/View' # for default_view, should fix this

# An actuator that can affect multiple SourceModifiers in response to changes in its own SourceModifier.
# On change of the bound SourceMOdifier, the bound SourceModifier's value is modified by a per-target mapping and then
# sent to the target SourceModifier. 
class ChangeActuator
	include Actuator
	include ConfigYAML
  include DRb::DRbUndumped
	include UniqueIDConfig
	include ObjectHost
	include SymbolDefaults

	attr_accessor :targets
	default :targets, {}
	
	persistent :targets
  default_view :targets, "::Views::HashObjectReferences"

	def initialize(owner=nil)
		initActuator(owner)
		@targets = {}
		symboldefaults_init(ChangeActuator)
	end

	def self.createDefaultWithOwner(owner, *args)
		ChangeActuator.new
	end
	
	def addTarget(target, mapping = nil)
		raise "Target must be a sourcemodifier (target is class #{target.class})" if !target.kind_of?(SourceModifier)
		
		mapping = nil
		
		begin
  		if !mapping
        mapping = if @sourceModifier
          DRb.send_authoritive(true, Mapping::Linear, :new, @sourceModifier.min, @sourceModifier.max, target.min, target.max)
  			else
          DRb.send_authoritive(true, Mapping::Linear, :new, 0.0, 1.0, target.min, target.max)
  			end
  		end
  		
  		updateTargets
  
  		@targets[target] = mapping
  		@targets.modified(self)
	  ensure
	    $djApp.release_authoritive(mapping)
	  end
	end

	def updateFromSource(value)
		updateTargets
		super
	end

	def updateTargets
		return if !@sourceModifier
		
		@targets.each do |t, m|
			t.set_value(m * @sourceModifier.value, self)
		end
	end
end
