require './Source/AudioLibrary'
require './Source/source'
require './Source/dsound/source_stream_dsound'
require './Source/master'
require 'std/ConfigSet'

class AudioLibraryDSoundPlatform < AudioLibrary
  abstract
  
  def initialize
    super
    @library = libraryClass.new(PlatformRuby.program_name)
    @library.setDesiredLatency(1.0 / 30.0)
    @library.open
  end
  
  def native_library_instance
    @library
  end
  
  def defaultDevice
    if !@defaultDevice
      @defaultDevice = @library.defaultDevice
    end
    
    @defaultDevice
  end

  def devices
    # Caching the variable prevents the returned value from being garbage collected, potentially causing
    # recycled object errors for drb clients. SWIG returns a new device list array each time.
    @devices ||= @library.devices
  end

  def latency
    @library.desiredLatency
  end
  
  def libraryClass
    raise "Supply a native AudioLibrary class."
  end
  
  def masterClass
    MasterDSound
  end
  
  def monitorClass
    MonitorDSoundDevice
  end
  
  def mp3StreamClass
    SourceStreamDSound
  end
  
  def refreshDevices
    @devices = nil
    @library.refreshDevices
  end
end

class AudioLibraryCoreAudio < AudioLibraryDSoundPlatform
  def libraryClass
    Dsound::CoreAudioLibrary
  end
end

class AudioLibraryDirectSound < AudioLibraryDSoundPlatform
	def libraryClass
		Dsound::DirectSoundLibrary
	end

  def master_window=(window_handle)
    @master_window_handle = window_handle
    @sources.each do |s|
      s.on_master_window_set(self)
    end
  end
end

class AudioLibraryPortAudio < AudioLibraryDSoundPlatform
  def libraryClass
    Dsound::PortAudioLibrary
  end
end

module OutputDSound
  attr_reader :device
  attr_reader :inputToDevice
  attr_reader :gainEffect
	
  def close
    if @device 
      @device.removeInput(@inputToDevice) if @inputToDevice
      @device = nil
      @inputToDevice = nil
    end
    
    super
  end
	
	def onDeviceChanging(d)
    @device.removeInput(@inputToDevice) if @inputToDevice && @device

    @device = d
		
    @inputToDevice = outputNode.requestOutput
		
    begin
      @device.addInput(@inputToDevice)
    rescue Exception=>e
      $stderr.puts "Error trying to open #{@device.description}: #{e}"
    end

		GC.start
	end
	
	def onLibraryInit(audioLibrary)
		super
	end
	
  def on_master_window_set(audio_library)
    log { "Setting master window handle: '#{audio_library.master_window_handle}'" }
    @device.setWindow(audio_library.master_window_handle)
  end
  
	def outputNode
		if !@outputNode
			@outputNode = Dsound::ModifierNode.new
			makeEffects(@outputNode)
		end
		
		@outputNode
	end
	
	def makeEffects(node)
    @limiter = Dsound::Limiter.new(Dsound::Gain.new(-1.0), 0.05, 0.1)
    node.addModifier(@limiter, 10)
    
    # Master gain must proceed encoder connected to master output, otherwise master gain changes will be propagated
    # to the output recorded mix.
    @gainEffect = Dsound::GainEffect.new
    node.addModifier(@gainEffect, 20)
	end

	def sourceOutput(source)
		source.output
	end

	def modifiable
		self.outputNode
	end

  # syncs periodically to the given output, or none if 'nil' is supplied
	def syncToOutput(outputDsound, period = 10.0)
    if outputDsound == nil
    	if @syncThread
	      @syncThread.requestExit
	      @syncThread = nil
	    end
    else
    	raise "syncToOutput has been called twice with different sync targets." if @syncThread
    	
      @syncThread = SafeThread.loop do
        if @device
          xfadeSource = outputDsound.modifierOfType(SourceModifiers::Crossfade)
          xfadeDest = modifierOfType(SourceModifiers::Crossfade)
          
          # Check xfade modifiers because this routine may be called before crossfade modifiers have been added.
          # tbd: add more correct enumeration of inputs -- it is not always valid to assume that all inputs are
          # attached to the crossfade modifiers.
          if xfadeSource && xfadeDest
            xfadeSource.inputsFromSources.zip(xfadeDest.inputsFromSources) do |source, dest|
              if outputDsound.device
                outputDsound.device.syncRequestAdd(source, dest, @device)
              end
            end
          end
        end
        
        sleep(period)
      end
    end
	end
		
	def visualisable
		outputNode.visualisable
	end

	def gain=(db)
		@gainEffect.setGain(db) if @gainEffect
	end
end

class MasterDSound < MasterOutput
  include EnumerableInstances
	include OutputDSound
	
	def initialize(app)
		super(app)
		@audioLibrary = app.audioLibrary
	end

  def close
    @encode.setEnabled(false)
    @encode.destroy
    super
  end

	def makeEffects(node)
		super
		
    outputFilePath().make_dirs
    
    # Encoder must precede master gain, otherwise master gain changes will be propagated
    # to the output recorded mix.
		#@encode = Dsound::EncodeMP3.new(@audioLibrary.native_library_instance, outputFilePath().to_sys)
    @encode = Dsound::EncodeFlac.new(@audioLibrary.native_library_instance, outputFilePath().to_sys)
    @encode.setEnabled(record())
		node.addModifier(@encode, 15)

		onRecordingInit
	end	

	def record=(status)
		super
		@encode.setPath(self.outputFilePath.to_sys)
		@encode.setEnabled(status)
	end
end

class MonitorDSoundDevice < MasterMonitor
	include OutputDSound
  include SymbolDefaults
  
  attr_reader :sync
  
  persistent_accessor :sync
  default :sync, true
	
	def initialize(app, master)
    super(app)
		@audioLibrary = app.audioLibrary
		@master = master
    symboldefaults_init(MonitorDSoundDevice)
	end

  def onDeviceChanged
    super
    self.sync = false
    self.sync = sync()
  end
  
	def sourceOutput(source)
		source.outputClean
	end
  
	def sync
	  false # sync is disabled
	end
	
  def sync=(b)
    raise "A master must be present." if Master.instances.empty?
    
    if b
      syncToOutput(MasterDSound.instances.last)
    else
      syncToOutput(nil)
    end
  end
  
  def makeEffects(node)
    super
    
    # apply the master's gain to the monitor, so that monitor gain becomes relative to master.
    node.addModifier(@master.gainEffect, 15)
  end
end

