require 'std/ConfigYAML'
require 'std/module_logging'
require 'thread'
require 'std/view/Viewable'

if Object.const_defined?(:Midiinterface)
  class Midiinterface::MIDIData
    def to_s
      "MIDIData id: #{id()}, data1: #{data1()}, data2: #{data2()}"
    end
  end
end

class MIDIDeviceIn
  include ModuleLogging
  MIDIDeviceIn.log=false
  
  attr_accessor :opened

	def initialize(app, nativeDevice)
		super()
		@listeners = []
		@data = []
		@lastTimestamp = nil
		@nativeDevice = nativeDevice
		@timestep_s = 1.0 / app.update_freq
  end

	def addListener(app, listener)
	  log { "Listener add request: #{listener}" }
    app.mainLoop.addUpdateClient(self) if @listeners.empty?
		@listeners << listener if !@listeners.include?(listener)
	end

	def removeListener(app, listener)
		@listeners.delete(listener)
    app.mainLoop.removeUpdateClient(self) if @listeners.empty?
    log { "Listener remove request: #{listener}" }
	end

	def data
		@nativeDevice.data.dup
	end

	def open
    @opened = true
		@nativeDevice.open
		@lastTimestamp = nil
	end
	
	def name
		@nativeDevice.name
	end
	
	def close
    @opened = false 
		@nativeDevice.close
	end

	def update(real_time)
		data = self.data()
    log { "Midi data: #{data.length} entries." if !data.empty? }

    if @listeners.empty?
      log { "No listeners." if !data.empty? }
      return 
    end

    until data.empty?
      if @lastTimestamp == nil
        @lastTimestamp = data.first.time
        @lastTimestampRelativeToRealTime = real_time - data.first.time
      end

      log { "Begin timestep at #{@lastTimestamp}." }

			# aggregate data

			# messages are grouped in timestep_ms blocks, so that non-linear processing on midi messages is deterministic.
			# otherwise, the results of such processing can vary depending on the gap between calls to update.
			allMessages = []
			currentBlock = []
			
			blockStartTime = @lastTimestamp
			blockEndTime = blockStartTime + @timestep_s
			blockPeriod = blockEndTime - blockStartTime

			while !data.empty?# && data[0].time <= blockEndTime do
				allMessages << data.shift
			end

			log { "#{allMessages.length} messages in timestep block." }

			if !allMessages.empty?
  			listenerMessages = []
  			
  			@listeners.each do |l|
          log { "Listener: #{l.class}" }
  				if !l.enabled?
            log { "  Disabled." }
  				  next
  				end
  			
  				allMessages.each do |m|
  					if l.triggeredBy?(m.id, m.data1)
              log { "  Triggered: #{m}." }
  						listenerMessages << m
  					else
              log { "  Not triggered: #{m}." }
  					end
  				end
  				
  				if !listenerMessages.empty?
  					l.trigger(listenerMessages)
  					l.postTrigger 
  					
  					if l.consume?(listenerMessages)
  						allMessages = allMessages - listenerMessages
  					end
  					
  					listenerMessages.clear
  				end
  			end
			end

			@lastTimestamp += blockPeriod
      @lastTimestampRelativeToRealTime += blockPeriod
      log { "End timestep at #{@lastTimestamp}." }
      log { "End." } if data.empty?
		end
	end
end

module MIDIMessage
	class Base
	  include DRb::DRbUndumped
		include ConfigYAML
		include Viewable

		persistent :matchId

		def initialize(matchId)
			@matchId = matchId
		end

		def matches?(messageId, data1)
			return messageId == @matchId
		end
	end
	
	class CC < Base
    include DRb::DRbUndumped
    
		attr_accessor :controlCode
		
		persistent :controlCode

		def initialize(channel, controlCode)
			super(0)
			@controlCode = controlCode
			self.channel=channel
		end
		
		def channel=(channel)
			@matchId = 0xb0 + (channel - 1)
		end
		
		def matches?(messageId, data1)
			super(messageId, data1) && data1 == @controlCode
		end
		
		def channel
			return @matchId - 0xb0
		end
	end
end


class MIDIMessageListener
	include ConfigYAML
	include Viewable
	
	attr_reader :message
	attr_accessor :block
	attr_accessor :postTriggerBlock

	no_view :block
  no_view :postTriggerBlock
	
	persistent :enabled
	persistent :message
	
	def initialize(message, block = nil, postTriggerBlock = nil)
		@message = message
		@block = block
		@postTriggerBlock = postTriggerBlock
		@enabled = true
		@consume = false
	end
	
	def consume=(b)
		@consume = b
	end

	def consume?(values)
		@consume
	end

	def enabled?
		@enabled
	end

	def enabled=(b)
		@enabled = b
	end
	
	def triggeredBy?(messageId, data1)
		return @message.matches?(messageId, data1)
	end
	
	def trigger(dataArray)
		@block.call(dataArray.collect { |a| a.data2.to_f })
	end

	def postTrigger
		@postTriggerBlock.call
	end
end
