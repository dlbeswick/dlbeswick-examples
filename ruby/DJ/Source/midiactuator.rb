require './Source/actuator'
require './Source/midi/midi'
require 'std/ConfigYAML'
require 'std/EnumerableSubclasses'
require 'std/mapping'
require 'std/SymbolDefaults'

# A MIDIActuator is an Actuator driven by a MIDI control.
module MIDIActuator
	
	class Base
		include Actuator
		include ConfigYAML
    include DRb::DRbUndumped
		include EnumerableSubclasses
    include SymbolDefaults
		
		attr_writer :deviceIn
		attr_writer :deviceOut
		attr_accessor :mapping
    attr_reader :listenerCC
	
    default :mapping, Mapping::Linear.new(0.0, 127.0, 0.0, 1.0)
  
		persistent :listenerCC
		persistent :mapping
		
		no_view :deviceIn
    no_view :deviceOut
		
    def self.authoritive_create_default
      self.new(nil, 1, 0)
    end
    
		def initialize(actuatorHost, channel, controlCode)
      symboldefaults_init(MIDIActuator)

			@listenerCC = MIDIMessageListener.new(MIDIMessage::CC.new(channel, controlCode))
			
			self.enabled = true

			initActuator(actuatorHost)
		end

		def ordering(rhs)
			if @listenerCC
				@listenerCC.message.controlCode <=> rhs.listenerCC.message.controlCode
			else
				super
			end
		end

		def alpha(value)
			minVal = min
			maxVal = max
			return minVal + (value - minVal) / (maxVal - minVal)
		end
		
		def deviceIn
			if self.actuatorHost
				self.actuatorHost.device
			else
				nil
			end
		end
		
		def deviceOut
			nil
		end
		
		def onMessage(values)
			self.sourceValue=interpretValue(values, @sourceModifier.value) if @sourceModifier
		end

		def consume=(b)
			@listenerCC.consume = b
		end

		def consume?(values)
			@listenerCC.consume?(values)
		end

		def enabled?
			@listenerCC.enabled
		end

		def enabled=(b)
			@listenerCC.enabled = b
		end
		
		def mapping
		  if @sourceModifier && @mapping.dstExtent == 0
				@mapping.dstMin = self.mappingDstMin
				@mapping.dstMax = self.mappingDstMax
			end
			
			@mapping
		end
		
		def min
			return 0.0
		end
		
		def max
			return 1.0
		end

		def onPostMessages
		end
		
		def actuator_connect(sourceModifier)
			super
			if !sourceModifier.nil? || !needs_source_modifier?
				setupListener()
				deviceIn.addListener($djApp, @listenerCC) if deviceIn()
			end
		end
		
		def disconnect
			super
			deviceIn.removeListener($djApp, @listenerCC) if deviceIn()
		end

		def loaded
			super
			setupListener
		end
	
		def sendLearnData
			data = learnData
			@deviceOut.shortMsgOut(data[0], data[1], data[2])
		end

		def reset
			updateDevice
		end
		
	protected
		def mappingDstMin
			@sourceModifier.min
		end

		def mappingDstMax
			@sourceModifier.max
		end
	
		def learnData
			return [0,0,0]
		end

		# value sent to controller on source modification
		def outputValue
		end

		def setupListener
			@listenerCC.block = Proc.new { |data| onMessage(data) }
			@listenerCC.postTriggerBlock = Proc.new { onPostMessages }
		end
	
		def updateDevice
			output = outputValue
			# temporarily disabled -- was causing play requests on open, looping back to midi in somehow
			#@deviceOut.shortMsgOut(0xb0 + @listenerCC.message.channel.to_i, @listenerCC.message.controlCode.to_i, output.to_i) if output
		end

		def interpretValue(values, sourceValue)
			return values.last
		end
	end

	class Absolute < Base
		def initialize(actuatorHost, channel, controlCode)
			super(actuatorHost, channel, controlCode)
		end
		
		def learnData
			return [0xb0 + @listenerCC.message.channel, @listenerCC.message.controlCode, Mapping.linear(@sourceModifier.value, min, max, @sourceModifier.min, @sourceModifier.max)]
		end

		def min
			return 0.0
		end
		
		def max
			return 127.0
		end

	protected
		def outputValue
			self.mapping * @sourceModifier.value
		end

		def interpretValue(values, sourceValue)
			self.mapping * values.last
		end
	end
	
	class Relative < Base
		def initialize(actuatorHost, channel, controlCode)
			super(actuatorHost, channel, controlCode)
		end
	
		def onMessage(values)
			super
#			updateDevice
		end

		def learnData
			return [0xb0 + @listenerCC.message.channel, @listenerCC.message.controlCode, rand(128)]
		end

		def min
			return 0.0
		end
		
		def max
			return 127.0
		end

	protected
		def mappingDstMin
			@sourceModifier.min / 10
		end

		def mappingDstMax
			@sourceModifier.max / 10
		end

		def outputValue
			Mapping.linear(@sourceModifier.value, @sourceModifier.min, @sourceModifier.max, min, max)
		end

		def interpretValue(values, sourceValue)
			value = 0.0
			
			values.each do |v| 
				if v > 63.0
					v = v - 128.0
				end
	
				value = value + v
			end
			
			increment = value
			increment = mapping() * Math.clamp(value.abs, min(), max())

			if (value > 0)
				value = sourceValue + increment
			else
				value = sourceValue - increment
			end
			
			return Math.clamp(value, @sourceModifier.min, @sourceModifier.max)
		end
	end

	class Trigger < Base
		persistent :triggerOnRelease
		
		def initialize(actuatorHost, channel, controlCode, triggerOnRelease=true)
			super(actuatorHost, channel, controlCode)
			@triggerOnRelease = triggerOnRelease
		end
		
		def onMessage(values)
			values.each do |v|
				trigger(triggered?(v))
			end
		end
	
		def trigger(on)
			if on
				self.sourceValue = self.mapping * 1.0
			else
				self.sourceValue = self.mapping * 0.0
			end
		end
		
	protected
		def outputValue
			return 0
		end
		
		def triggered?(value)
			if @triggerOnRelease
				value == 0
			else
				value != 0
			end
		end
	end
	
	class Activate < Trigger
		def trigger(on)
			@sourceModifier.active=(on)
		end
	end
	
	# activates/deactivates another control on trigger
	class ActivateControl < Trigger
		persistent :target

		def initialize(actuatorHost, channel, controlCode, targetControl=nil, triggerOnRelease=true)
			super(actuatorHost, channel, controlCode, triggerOnRelease)
			
			@target = targetControl
		end

		def target=(actuator)
			@target=actuator
			@target.enabled = triggerOnRelease
		end

		def trigger(on)
			@target.enabled = on
		end
	end
	
	class TimeShuttle < Relative
		def actuator_connect(sourceModifier)
			super
			raise "TimeShuttle sourceModifier is of type '#{sourceModifier.viewable_class.name}', but can only operate on Time" if sourceModifier && !sourceModifier.viewable_kind_of?(SourceModifiers::Time)
		end
	
		def onMessage(values)
			@sourceModifier.cuePos=interpretValue(values, @sourceModifier.cuePos)
		end

		def onPostMessages
			@sourceModifier.cuePreview
		end
	end

	class CueShuttle < TimeShuttle
		def value=(value)
			@sourceModifier.cuePos = value
			super
		end
	end

	class PlayButton < Trigger
		def trigger(on)
			return if !on

			if self.source.playing
				self.source.pause
			else
				self.source.play
			end
		end
	end

	class CueButton < Trigger
		def trigger(on)
			return if !on

			@sourceModifier.cue
			onModifiedSource
		end
	end

	class ValueModifyButton < Trigger
		def initialize(actuatorHost, channel, controlCode)
			super(actuatorHost, channel, controlCode, false)
		end

		def trigger(on)
			if on
				self.sourceValue += mapping() * 1.0 
			else
				self.sourceValue -= mapping() * 1.0
			end
		end
	end

  class TrackSelection < Trigger
    include Abstract

		def initialize(actuatorHost, channel, controlCode)
			super(actuatorHost, channel, controlCode, false)
		end

    def iterator
      actuatorHost().track_iterator
    end

    def play
      return if iterator().current.nil?

      target_sources = $djApp.sources
      
      non_focused_source = target_sources.find { |source| !source.drb_equal?($djApp.focused_source) }
      
      if non_focused_source
        non_focused_source.open(iterator().current) 
        #SafeThread.new { non_focused_source.position = 60000; non_focused_source.play; }
        non_focused_source.play
      end
    end

    def needs_source_modifier?
      false
    end
  end

  class TrackNext < TrackSelection
		def trigger(on)
      if on && !iterator().nil?
        iterator().next

        if iterator().current.nil?
          iterator().start
        end

        play()
      end
    end
  end

  class TrackPrev < TrackSelection
		def trigger(on)
      if on && !iterator().nil?
        iterator().prev
        play()
      end
    end
  end
end
