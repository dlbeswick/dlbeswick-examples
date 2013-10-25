# The Master class takes input from several Sources and processes them (i.e. by applying crossfade)
# before sending the result to its output (typically a sound device).
# The Master is the final link in the output chain.
require './Source/source'
require 'std/ConfigYAML'

# Master shouldn't really inherit from Source. That'll fix being able to drop tracks on it.
class Master < Source
  include ConfigYAML
  include EnumerableInstances

  attr_reader :sources
  attr_reader :audioLibrary
  attr_reader :soundDeviceName
  
  persistent :soundDeviceName
  persistent :initialGain
		
  def initialize(app, name)
    super(name, app.audioLibrary)

    @audioLibrary = app.audioLibrary
    
    @initialGain = 1.0
  end

  # Master shouldn't really inherit from Source. That'll fix being able to drop tracks on it.
  def accepts_tracks?
    false
  end
	
  def close
    gainModifier = modifierOfType(SourceModifiers::Gain)
    @initialGain = gainModifier.value if gainModifier
    save
  end

	def addModifier(modifier)
		super
		modifier.value = @initialGain if modifier.kind_of?(SourceModifiers::Gain)
	end

	def device
		@device = @audioLibrary.defaultDevice if !@device
		@device
	end

	def device=(d)
    if d
      newDevice = d
    else
      log { "No device given, setting to default device '#{@audioLibrary.defaultDevice.description}'" }
      newDevice = @audioLibrary.defaultDevice
    end
	
    log { "Sound device set to '#{newDevice.description}'" }
      
    changed = @device != newDevice
		
    onDeviceChanging(newDevice) if changed

		@soundDeviceName = newDevice.description
		@device = newDevice

		onDeviceChanged if changed
	end

  def device_by_name=(device_name)
    return if @device.description == device_name
    device = $djApp.audioLibrary().devices.find { |device| device.description == device_name }
      
    raise "No such device '#{device_name}'" if !device
    self.device = device
  end
  
	def onDeviceChanging(d)
	end

  def onDeviceChanged
  end
  
	def onLibraryInit(audioLibrary)
		super
		setDefaultDevice
	end

	def setDefaultDevice
		return if !@audioLibrary.devices
		log { "Setting default device. Preference is '#{@soundDeviceName}'. Available devices:" }
		
		@audioLibrary.devices.each do |d|
		  log { d.description }
		end
		  
		self.device = @audioLibrary.devices.detect { |d| d.description == @soundDeviceName }
	end

	def onSourceConnected(source)
	end

	def removeSource(source)
		sources.delete(source)
	end
	
	def newStream(path)
		return nil
	end

	def gain=(db)
	end
end

# The MasterOutput is the final mixed output from the program.
class MasterOutput < Master
	attr_reader :record

	def initialize(app)
		@record = false
		@recordingDate = Time.now
		super(app, "Master")
	end
	
	def outputFilePath
		return $djApp.recordingPath.with_file(@recordingDate.strftime('%Y %m %d %H %M %S'))
	end

	def record=(status)
		@record = status
	end

protected
	def onRecordingInit
	end
end

# The MasterMonitor is the output to the headphone monitor.
class MasterMonitor < Master
	def initialize(app)
		super(app, "Monitor")
	end
end
