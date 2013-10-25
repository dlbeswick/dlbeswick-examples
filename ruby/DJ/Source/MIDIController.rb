require './Source/ActuatorHost'
require 'std/ConfigYAML'
require 'std/view/Viewable'

class MIDIController < ActuatorHost
	attr_reader :device
	
	persistent :deviceName

  attr_model_value :hostedActuators do 
    obj = MIDIActuator::Absolute.new(nil, 1, 1)
    obj.actuatorHost = self
    [obj]
  end

	def initialize(device=nil)
		super()
		
		if device
			self.device=device 
		end
		
		symboldefaults_init(MIDIController)
	end

  def track_iterator=(music_set_iterator)
    @track_iterator = music_set_iterator
    @track_iterator.start
  end

  def track_iterator
    @track_iterator
  end
	
	def hostActuator(actuator, instigator, sort=true, updateObjList=true)
		actuator.listenerCC.message.channel = 1
		actuator.deviceIn = @device
		super
	end

	def device=(device)
		return if @device == device
		@device = device
	  log { "Opening #{device.name}." }
		if @device
			@deviceName = @device.name
			@device.open
      log { "Opened." }
		else
			@deviceName = ''
		end
		updateActuators
	end

  def device_by_name=(device_name)
    return if @device && @device.name == device_name
    device = $djApp.midiDevicesIn.find { |device| device.name == device_name }
      
    raise "No such device '#{device_name}'" if !device
    self.device = device
  end
  
	def updateActuators
		self.hostedActuators.each do |a|
			a.deviceIn = @device
		end

		super
	end
	
	def viewableName
		return 'MIDI Controller' if !@deviceIn
		@deviceIn.name
	end
	
protected
  def loaded
    super
    
    self.device = $djApp.midiDevicesIn.detect { |d| d.name == @deviceName }
  end
end
