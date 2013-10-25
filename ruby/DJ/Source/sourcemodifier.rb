require './Source/ActuatorHost'
require './Source/master'
require 'std/ConfigYAML'
require 'std/SymbolDefaults'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'
require 'std/EnumerableInstances'
require 'std/math'
require 'mutex_m'
require 'set'

class SourceModifierBulkNotifier
  include NotifyHost

  attr_accessor :updates
  
  class Notify < Notify::Callback
  end

  def initialize
    @updates = {}
    @updates.extend(Mutex_m)
    
    updates_current = []
      
    SafeThread.loop do
      @updates.synchronize do
        @updates.each do |key, value|
          updates_current << [key, value]
        end
        
        @updates.clear
      end
      
      if !updates_current.empty?
        notify_call(Notify, self, updates_current)
        updates_current.clear
      end

      sleep(0.1)
    end
  end  
end

# applies the operation given in the block to a group of source modifiers.
class SourceModifierBatchUpdate
  def initialize(source_modifiers, instigator, &block)
    @updates = Set.new
    
    source_modifiers.each do |modifier|
      yield modifier
      @updates << modifier
    end
    
    finalize(instigator)
  end

:protected  
  def finalize(instigator)
    sources = Set.new 
    
    @updates.each do |source_modifier|
      source_modifier.onChange(instigator, false)
      sources.merge(source_modifier.sources)
    end
    
    sources.each do |source|
      source.modify
      source.commit
    end
  end
end

# A SourceModifier makes changes to the output of sources.
class SourceModifier < ActuatorHost
	include EnumerableInstances
	include NotifyHost
	include ConfigYAML
  include SymbolDefaults
	include UniqueIDConfig
	include Viewable
	
	attr_reader :owner
	
	default :actuators, []
	
	persistent :sources
	persistent :value
	
	class NotifyChange < Notify::Callback
	end

public
	def initialize(owner, value, sources = nil)
	  self.ensure_id
		@value = value
		@value_mutex = Mutex.new
 
    @actuators_needing_update = []
    
		sources = Array(sources)

		if !sources[0]
			@sources = []
		else
			@sources = sources.flatten
		end
		
		if @sources.empty? then
			@sources << owner 
		end
		
		@owner = owner
		@owner.addModifier(self)
		@defaultValue = value
		@links = []
    
    symboldefaults_init(SourceModifier)
    
    @actuators.extend(Mutex_m)
    @sources.extend(Mutex_m)
	end

	def actuators
	  @actuators.synchronize do
	    @actuators.dup
	  end
	end
	
	def onActuatorConnected(actuator)
    @actuators.synchronize do
  		if !@actuators.include?(actuator)
  		  @actuators << actuator
  			
  			if actuator.update_on_source_modify?
  			  @actuators_needing_update << actuator
  			end
  		end
    end
	end

	def onActuatorDisconnected(actuator)
	  @actuators.synchronize do
  		@actuators.delete(actuator)
      @actuators_needing_update.delete(actuator)
	  end
		modify
	end

	def display(value = @value)
		format('%.4f', v)
	end

	def min
	end
	
	def max
	end
	
	def modify
		doModify
	end

	def onChange(instigator, updateSources = true)
    if updateSources
      @sources.each do |s|
        s.modify
        s.commit
      end
    end
    
    v = value()
	  
    bulk_updates = $djApp.sourcemodifier_bulk_notifier.updates()
    @actuators_needing_update.each do |a|
      if a != instigator
        # tbd: still needed? yes, because bulk updates should only be necessary for remote actuators.
        # should be implemented better though.
        if a.kind_of?(DRb::DRbObject) || a.kind_of?(DRb::DRbUnknown)
          bulk_updates.synchronize do
            bulk_updates[a] = v
          end
        else
          a.updateFromSource(v)
        end
      end
		end
		
		notify_call(NotifyChange, instigator)
	end
	
	def onSourceClose
	end
	
	def set_defaults
    @value = @defaultValue
	end
	
  def sources
    #fix
    if @sources && !@sources.class.include?(Mutex_m)
      @sources.extend(Mutex_m)      
    end
    
    @sources.synchronize do
      @sources.dup
    end
  end

	def value
    @value_mutex.synchronize do
      @value
    end
	end
	
	def value=(v)
	  set_value(v, nil)
	end
	
	def set_value(v, instigator)
    changed = false
    
    @value_mutex.synchronize do
      changed = @value != v
      @value = v
    end
    
    if changed
      onChange(instigator)
    end 
    
    nil
	end
	
	def viewable_name
		names = sources().collect { |n| n.name }
		"#{super} #{self.unique_id}"
	end
	
	def viewable_description
		names = sources().collect { |n| n.name }
		"hosted by source '#{names.join(', ')}'"
	end

	def active=(b)
	end
	
protected
	def doModify
	end
end

module SourceModifiers
	class Effect < SourceModifier
		def initialize(owner, value, priority = 1)
			@effect = self.makeEffect

			super(owner, value)
		end

		# The "clean" output has effects applied that are inherent to how the stream sounds, such as
		# non-crossfade related filtering and stream gain.
		# It's sent to the monitor.
		def addClean
      owner.outputClean.addModifier(@effect)
      self
		end
		
    # The "modifiable" output has effects applied that affect the final mix as a result of dj-ing, such as
		# crossfade.
    # It's sent to the master output.
		def addModifiable
      owner.modifiable.addModifier(@effect)
      self
		end

		def addEffectToNode(node)
      node.addModifier(@effect)
		end

		def effect
			@effect
		end

		def makeEffect
			raise 'Must return an effect.'
		end
	end
	
	class Pitch < Effect
		def initialize(owner)
			super(owner, 100.0)
		end
	
		def makeEffect
			Dsound::Frequency.new
		end
		
		def min
			return 0.0001
		end

		def max
			return 300.0
		end
		
		def doModify
          scale = 
            if !owner().open?
              1.0
            else
              rate = owner().sample_rate

              # correct here for input sample rates other than 44100
              # tbd: correct to output sample rate other than 44100 (no hardcode)?
              sample_rate_adjust = 
                if rate != 0
                  rate / 44100.0
                else
                  1.0
                end
              
              (self.value / 100.0) * sample_rate_adjust
            end

          @effect.frequencyScale.set(scale)
		end
		
		def display(value)
			result = format('%.4f%%', value)
			result
		end
	end

	class Gain < SourceModifier
		def initialize(owner, value = 0.0)
			super
		end

		def display(value)
			format('%.1fdb', value)
		end
    
		def min
			return -144.0
		end

		def max
			return 60.0
		end

		def doModify
			sources[0].gain += value()
		end
	end
	
	class Crossfade < Effect
    attr_reader :inputFromSourceA
    attr_reader :inputFromSourceB
    attr_reader :inputsFromSources
    
		def initialize(owner, sourceA, sourceB, use_clean)
			super(owner, 0.0)
      
      @xfadeSources = [sourceA, sourceB]
      
      @gains = [Dsound::GainEffect.new, Dsound::GainEffect.new]
      @nodes = [Dsound::ModifierNode.new, Dsound::ModifierNode.new]
      
      @inputFromSourceA = if use_clean
        sourceA.outputClean.requestOutput
      else
        sourceA.output.requestOutput
      end
      @nodes[0].addInput(@inputFromSourceA)
      @nodes[0].addModifier(@gains[0])

      @inputFromSourceB = if use_clean
        sourceB.outputClean.requestOutput
      else
        sourceB.output.requestOutput
      end
      @nodes[1].addInput(@inputFromSourceB)
      @nodes[1].addModifier(@gains[1])
      
      @inputsFromSources = [@inputFromSourceA, @inputFromSourceB]
      
      @nodes.each do |n|
        @effect.add(n.requestOutput)
      end

      unattenuatedPosition = 76.0 / 127
      quietGain = -6.0
      quietPosition = 96.0 / 127
      inaudibleGain = -24.0
      inaudiblePosition = 125.0 / 127
      minGain = -144.0
      
      dataPoints = [
        [0.0, 0.0],
        [unattenuatedPosition, 0.0],
        [quietPosition, quietGain],
        [inaudiblePosition, inaudibleGain],
        [1.0, minGain]
      ]
        
      @gainFunctions = []
      
      @gainFunctions << begin
        # tbd: this method should not modify the array!
        Mapping::DataPoints.absolute(dataPoints)
      end

      @gainFunctions << begin
        inverseDataPoints = dataPoints.collect do |tuple|
          [1.0 - tuple.first, tuple.last]
        end
        inverseDataPoints.sort_by! { |tuple| tuple.first }

        Mapping::DataPoints.absolute(inverseDataPoints)
      end
    end
    
    def makeEffect
			Dsound::MixGeneratorOutputs.new
    end

		def min
			return 0.0
		end

		def max
			return 1.0
		end

		def display(value)
      if value == min()
        @xfadeSources[0].name
      elsif value == max()
        @xfadeSources[1].name
      else
        super
      end
		end

		def doModify
      v = value()
      @gains.each_with_index do |effect, i|
        effect.setGain(@gainFunctions[i] * v)
      end
		end
	end
	
	class Time < SourceModifier
		attr_reader :cuePos
		
		def initialize(owner, *sources)
			super(owner, 0.0, sources)
			
			@thread = SafeThread.new {
				sleep rand
				loop {
					sleep 1
					updateFromStream if !self.sources.empty? && self.sources[0].playing
					break if SafeThread.exit?
				}
			}
			
			@dirty = false
			@cuePos = 0.0
			@cuePreviewLength = 100.0

			@sources.each do |s|
				s.notify_listen( Source::NotifyPositionChange.new(self) { updateFromStream } )
			end
		end

		def onSourceClose
			super
		end

		def min
			return 0.0
		end

		def max
		  if @sources[0].mp3
        @sources[0].mp3.lengthMS
		  else
		    0.0
		  end
		end

		def display(value)
			pitch = @sources[0].modifierOfType(SourceModifiers::Pitch).value
			return "0" if pitch == 0 || sources[0].length == 0
			msPosition = (value * (100.0 / pitch)).to_i
			return "#{msPosition / 1000 / 60}:#{format("%02i",(msPosition / 1000) % 60)}:#{format("%03i",msPosition % 1000)}"
		end

		def doModify
			if @dirty
				sources[0].position = @value
				@dirty = false
			end
		end
		
    def set_defaults
      @cuePos = 0.0
    end
    
		def set_value(v, instigator)
			@dirty = true
      super
		end
		
		def updateFromStream
			@value = sources[0].position
			onChange(self, false)
		end

		def cue
			set_value(@cuePos, nil)
		end
		
		def cuePreview
			sources[0].playSegment((@cuePos..(@cuePos + @cuePreviewLength)))
		end

		def cuePos=(v)
			@cuePos = Math.clamp(v, min(), max())
		end

		def onSourceReset
			super
      if defaultOnSourceReset()
        @cuePos = 0.0
      end
		end
	end

#	class Filter < Effect
#		def initialize(kernelFunc, owner, value, priority = 1)
#			@kernelFunc = kernelFunc
#			@kernel = Dsound::FilterKernelFFT.new(@kernelFunc)
#			super(owner, value, priority)
#		end
#	
#		def min
#			return 0.0
#		end
#
#		def max
#			return 1.0
#		end
#		
#		def doModify
#		end
#
#		def makeEffect
#			Dsound::Filter.new(@kernel)
#		end
#	end
#
#	class BlackmanHighpass < Filter
#		def initialize(owner, cutoff, bandwidth, priority = 1)
#			super(Dsound::BlackmanSincLowpass.new(mapCutoff(cutoff), mapBandwidth(bandwidth)), owner, cutoff, priority)
#		end
#	
#		def mapCutoff(v)
#			Mapping.linear(v, 0.0, 1.0, 0.0000000000000001, 0.5)
#		end
#		
#		def mapBandwidth(v)
#			Mapping.linear(v, 0.0, 1.0, 0.0000000000000001, 0.5)
#		end
#		
#		def doModify
#			@effect.setEnabled(self.value > 0.0)
#			
#			if @effect.enabled
#				newCutoff = mapCutoff(self.value)
#			
#				if newCutoff != @kernelFunc.cutoff
#					@kernelFunc.cutoff = newCutoff
#					@kernel.eval
#				end
#			end
#		end
#		
#		def display(value)
#			if (value == 0.0)
#				return "Highpass off."
#			else
#				return format('Highpass %.2f%%', value * 100.0)
#			end
#		end
#	end
#	
	class Filter < Effect
		def min
			0.0
		end
		
		def max
			1.0
		end
	end
	
	class Lowpass < Filter
		def makeEffect
			Dsound::FilterLowpass.new
		end
	
		def cutoff=(v)
			@effect.cutoff = v
			@effect.setEnabled(v != 0.0)
		end
		
		def doModify
		  @mapping ||= Mapping::Log.new(min(), max(), 1.0, 0.0)

		  new_value = if value() == 0
        1.0
      elsif value() == 1
        0.0
      else
        @mapping.calc(value())
      end
      
			self.cutoff = new_value
		end
		
		def display(value)
			if (value == 0.0)
				return "Lowpass off."
			else
				return format('Lowpass %.2f%%', value * 100.0)
			end
		end
	end

	class Highpass < Filter
		def makeEffect
			Dsound::FilterHighpass.new
		end
	
		def cutoff=(v)
			@effect.cutoff = v
			@effect.setEnabled(v != 0.0)
		end
		
		def doModify
      @mapping ||= Mapping::Log.new(min(), max(), 0.0, 1.0)
      
      new_value = if value() == 0
        0.0
      elsif value() == 1
        1.0
      else
        @mapping.calc(value())
      end
      
      self.cutoff = new_value
		end
		
		def display(value)
			if (value == 0.0)
				return "Highpass off."
			else
				return format('Highpass %.2f%%', value * 100.0)
			end
		end
	end

	class Hold < Effect
		def initialize(owner, seconds, priority = 1)
			super(owner, seconds, priority)
		end

		def makeEffect
			Dsound::Hold.new
		end

		def active=(b)
			if b
				@effect.hold
			else
				@effect.release
			end
		end
		
		def min
			0.0
		end
		
		def max
			@effect.maxHoldSeconds
		end
		
		def doModify
			@effect.setHoldSeconds(self.value)
		end
	end
end
