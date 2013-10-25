require './Source/djinit'
require './Source/AudioLibrary'
require './Source/midi/midi'
require './Source/midiactuator'
require './Source/MIDIController'
require './Source/musiccollection'
require './Source/sourcemodifier'
require './Source/analysis/event_database'
require './Source/dsound/rdsound'
require './Source/music_set_iterator_random'
require './Source/music_set_iterator_concat'
require './Source/music_set_iterator_sequential'
require 'std/Config'
require 'std/ConfigYAML'
require 'std/view/Viewable'
require 'std/progress'

#UniqueID.log = true
#UniqueIDConfig.log = true
#Viewable.log=true
#MIDIDeviceIn.log=true

$use_event_db = false

if $debug_ruby19_mingw
  $: << "."
  $: << "d:/depot/projects/ruby"
  $: << "d:/depot/projects/ruby/std"
  
  require 'std/platform_os'
  
  $: << "d:/depot/projects/common"
  $: << "d:/src/rubyinstaller/sandbox/ruby19_mingw/bin"
  PlatformOS.require_dylib('msvcrt-ruby191', false, false)
end

Debugger.settings[:autolist] = 1 if respond_to?(:Debugger)

$profileMemory = false
if $profileMemory
	require 'MemoryProfiler'
end

$profile = false
if $profile
	require 'ruby-prof'
end

class MainLoop
	def initialize
		@updateClients = []
	end
	
	def processUpdateClients(real_time)
		@updateClients.each do |o|
			o.update(real_time)
		end
	end
	
	def addUpdateClient(client)
		raise "Update client #{client} added twice" if @updateClient && @updateClient.include?(client)
		@updateClients << client
	end
	
	def removeUpdateClient(client)
		@updateClients.delete(client)
	end
end

class LockedTimestepLoop < MainLoop
  def initialize(updateFreq, priority = 0, &block)
    super()
    @updateFreq = updateFreq
    @priority = priority
    @updateTimestep = 1.0 / @updateFreq
    @accumulator = 0
    @timer = Timer::Timer.new
    run(block) if block
  end
  
  def join
    raise "'run' not yet called." if !@thread
    @thread.join
  end
  
  def run(&block)
    #do a sin calc updating the crossfader each tick
    
    @thread = SafeThread.new { 
      until SafeThread.exit?
        step(&block)
      end
    }
    
    @thread.priority = @priority
  end

  def step(&block)
    startTime = @timer.realTime
    
    block.call if block
    processUpdateClients(startTime)
    
    sleep_period = @updateTimestep - Math.min(@updateTimestep, ((@timer.realTime - startTime) + @accumulator))
    sleep(sleep_period)
    
    @accumulator = (@timer.realTime - startTime) % @updateTimestep
  end
  
  def stop
    @thread.request_exit
  end
end

class DJApp
	include ConfigYAML
	
	UPDATEFREQ = 30

	attr_reader :audioLibrary
	attr_reader :appPath
	attr_reader :dev
  attr_reader :event_database
  attr_reader :event_database_session
	attr_reader :mainLoop
  attr_reader :master
  attr_reader :midi
  attr_reader :monitor
  attr_reader :sourceA
  attr_reader :sourceB
  attr_reader :sourcemodifier_bulk_notifier
	attr_reader :midiDevicesIn
	attr_reader :musicCollections
	attr_reader :sampleRate
	attr_reader :recordingPath
	
	# Dsound::Gain instance defining gain applied when no normalization data is present 
	# for a track.
  attr_reader :volume_normalization_default
	
	persistent :audioLibrary
	persistent :dev
	persistent :sampleRate
	persistent :recordingPath
  persistent :config_volume_normalization_default

  default :config_volume_normalization_default, -10.0
  default :volume_normalization_default do Dsound::Gain.new(-10.0); end 
  
  # argv_opts: An args hash from Trollop.
	def initialize(argv_opts)
	  symboldefaults_init(DJApp)
	  
		MemoryProfiler.start if $profileMemory

    process_argv(argv_opts)
		
		@mainLoop = LockedTimestepLoop.new(UPDATEFREQ, 100)
		
    @sourcemodifier_bulk_notifier = SourceModifierBulkNotifier.new

		@appPath = Path.new(Dir.getwd)
		@recordingPath = @appPath + "Output-nobackup"

		@sampleRate = 44100

		# hacks to create config
		begin
		  load
		rescue DLB::Config::ExceptionLoad
		end

		@musicCollections = MusicCollectionList.new
		
    if !@audioLibrary
      raise "No audiolibrary classes found." if AudioLibrary.subclasses.empty?
		  
      AudioLibrary.subclasses.each do |c|
        next if c.abstract?
        
        begin
          puts "No audiolibrary present, trying to initialize #{c.name}"
          @audioLibrary = c.new
          puts "Success."
          break
        rescue NameError=>e
          puts "Failed (#{e})."
        end
      end
		end

    raise "No audiolibrary could be initialized for this platform." if !@audioLibrary
    @audioLibrary.initLibrary(nil)
    
    begin
		  initMidi
    rescue Exception=>e
      # tbd: add this to the master 'error log' for later retrieval by the gui
      puts "Problem initializing midi -- midi not available."
      puts "Error detail: #{e}"
    end
		
		self.recordingPath.make_dirs
		self.configPath.make_dirs
		
		if $use_event_db
  		Progress.run("Loading event database") do
  		  @event_database = EventDatabase.new
  		  #begin
  		  #  @event_database.load
  		  #rescue DLB::Config::ExceptionLoad
  		  #end
        @event_database_session = @event_database.new_session
        @event_database.save_session(@event_database_session)
  		end
		end
	end

  def process_argv(argv_opts)
    @playlists_user = (argv_opts[:playlist] || []).collect do |path|
      MusicCollectionM3U.new(Path.to_file(path))
    end
    
    @run_gui = argv_opts[:run_gui]
  end

	def close
      if $profile
        printer = RubyProf::GraphHtmlPrinter.new(RubyProf.stop)
        
        printer.print(File.new("profiler.html", "w"), :min_percent=>2)
        puts "Wrote profiler data to 'profiler.html'"
      end
      
      if @event_database
        Progress.run("Closing event database") do
          @event_database.close
        end
      end
      
      Resource::Base.saveCache # should go after all this. shutdown is having issues, so do it here for now.
      @audioLibrary.close if @audioLibrary
      SafeThread.exitAll
      
      if $profileMemory
        MemoryProfiler.stop
      end
	end

	def configFilePath
		raise "No appPath" if !@appPath
		return @appPath + "Config"
	end

	def dev=(b)
		@dev = b
		DevContainer.instances.each do |d|
			d.updateFromApp
		end
	end

	def hello
	  "Hello."
	end
	
	def pid
	  Process.pid
	end
	
	def initMidi
    @midiDevicesIn = []
		@nativeMidiDevices = Midiinterface::PlatformMIDIDevices.new(PlatformRuby.program_name)
		
		@nativeMidiDevices.devicesIn.each do |o|
			@midiDevicesIn << MIDIDeviceIn.new(self, o)
		end
	end
	
	def midi_shutdown
    @nativeMidiDevices.close
	end
	
  def profile
    SafeThread.new do
      RubyProf.start
      sleep 15
      result = RubyProf.stop
      puts "Stopped profiling."
      printer = RubyProf::GraphHtmlPrinter.new(result)
      
      printer.print(File.new("profile_app.html", "w"), :min_percent=>2)
      puts "Wrote profiler data to 'profile_app.html'"
    end
  end

	def quit
	end

	def tempSetup
	  # tbd: use new 'unique' class to give important objects unchangable ids, rather than the auto_id counter
		oldUniqueID = UniqueID.auto_id
	
	# create master
    UniqueID.auto_id = 0

    log { "Creating instance of Master audio class '#{audioLibrary().masterClass}'" }
		@master = audioLibrary().masterClass.new(self)
		begin
		  @master.load
		rescue DLB::Config::ExceptionLoad
		end
    @master.setDefaultDevice
		
	# create monitor
    UniqueID.auto_id = 201
		@monitor = audioLibrary().monitorClass.new(self, @master)
		begin
		  @monitor.load
    rescue DLB::Config::ExceptionLoad
    end
    @monitor.setDefaultDevice
    
	# create sources
    UniqueID.auto_id = 204
		@sourceA = audioLibrary().mp3StreamClass.new(self, "A")
	
    UniqueID.auto_id = 205
		modifier = SourceModifiers::Gain.new(@sourceA, 0.0)
    modifier.value = -6.0
	
    UniqueID.auto_id = 206
    modifier = SourceModifiers::Pitch.new(@sourceA).addClean
    @sourceA.modifier_should_set_defaults_on_open(modifier, false)
	
    UniqueID.auto_id = 207
		SourceModifiers::Lowpass.new(@sourceA, 1.0).addClean
	
    # highpass used to automatically moderate bass during crossfade. not sent to monitor.
    UniqueID.auto_id = 208
		SourceModifiers::Highpass.new(@sourceA, 0.0).addModifiable
	
    UniqueID.auto_id = 231
    SourceModifiers::Highpass.new(@sourceA, 0.0).addClean
    
    UniqueID.auto_id = 209
		SourceModifiers::Time.new(@sourceA)
	
    UniqueID.auto_id = 210
		SourceModifiers::Hold.new(@sourceA, 0.06).addModifiable
		
    UniqueID.auto_id = 211
		@sourceB = audioLibrary.mp3StreamClass.new(self, "B")
	
    UniqueID.auto_id = 212
    modifier = SourceModifiers::Gain.new(@sourceB, 0.0)
    modifier.value = -6.0
	
    UniqueID.auto_id = 213
    modifier = SourceModifiers::Pitch.new(@sourceB).addClean
    @sourceB.modifier_should_set_defaults_on_open(modifier, false)
	
    UniqueID.auto_id = 214
		SourceModifiers::Lowpass.new(@sourceB, 1.0).addClean
	
    # highpass used to automatically moderate bass during crossfade. not sent to monitor.
    UniqueID.auto_id = 215
		SourceModifiers::Highpass.new(@sourceB, 0.0).addModifiable
	
    UniqueID.auto_id = 233
    SourceModifiers::Highpass.new(@sourceB, 0.0).addClean
  
    UniqueID.auto_id = 216
		SourceModifiers::Time.new(@sourceB)
	
    UniqueID.auto_id = 217
		SourceModifiers::Hold.new(@sourceB, 0.06).addModifiable

  # add monitor effects, must be done after sources are defined
    UniqueID.auto_id = 202
		crossfade = SourceModifiers::Crossfade.new(@monitor, @sourceA, @sourceB, true)
		crossfade.addModifiable
  
		# note: monitor gain is expressed relative to master gain.
		# this frees the user from having to adjust headphone volume each time master volume is adjusted. 
    UniqueID.auto_id = 203
		modifier = SourceModifiers::Gain.new(@monitor)
    modifier.value = 0.0
	
	# add master effects, must be done after sources are defined
    UniqueID.auto_id = 219
		SourceModifiers::Highpass.new(@master, 0.0).addEffectToNode(@sourceA.modifiable)
	
    UniqueID.auto_id = 220
		SourceModifiers::Highpass.new(@master, 0.0).addEffectToNode(@sourceB.modifiable)
	
    UniqueID.auto_id = 218
		@crossfade = SourceModifiers::Crossfade.new(@master, @sourceA, @sourceB, false)
    @crossfade.addModifiable
	
    UniqueID.auto_id = 200
		modifier = SourceModifiers::Gain.new(@master)
    modifier.value = -25.0
    
	# fix: replace with standard point of loading config data into resources.
	# may perhaps be fixed when all objects are data-driven.
		SourceModifier.instances.each do |i|
		  begin
		    i.load
      rescue DLB::Config::ExceptionLoad
      end
		end

	# create midi
		@midi = MIDIController.new
		@midi.bind_id(100)
	
		UniqueID.auto_id = oldUniqueID

		begin
		  @midi.load
		rescue DLB::Config::ExceptionLoad
		end

		UniqueIDConfig.ensure_resolution(nil)
    
    baseMCPath = if PlatformOS.osx?
      Path.new('/Volumes/HD/david-data')
    elsif PlatformOS.windows?
      Path.new('d:/data')
    else
      result = Path.new('/media/data/Data')
      
      result = Path.new('/clone/desktop') if !result.exists?
      
      result
    end
    
    $quick_musiccollection = false
    if $quick_musiccollection
      @musicCollections = MusicCollectionList.new
      @musicCollections << MusicCollectionPath.new(baseMCPath + "music/mp3/The Horrorist")
    else
      main_collection = 
        if PlatformOS.osx?
          MusicCollectionPath.new(baseMCPath + "music")
        elsif PlatformOS.windows?
          MusicCollectionPath.new([baseMCPath + "music", baseMCPath + "music-transcoded"])   
        else
          MusicCollectionPath.new([baseMCPath + "music"])   
        end
        
      selective_collection = 
        if PlatformOS.osx?
          nil
        elsif PlatformOS.windows?
          MusicCollectionM3U.new(baseMCPath + "playlists/No Crap.m3u")
        elsif PlatformOS.linux?
          MusicCollectionM3U.new(Path.to_file('/home/david/playlists/all-minus-excluded.m3u'))
        end

      @musicCollections << main_collection
      @musicCollections << selective_collection
    end

    if !@playlists_user.empty?
      @musicCollections.concat(@playlists_user)

      playlist_iterators = @playlists_user.collect do |playlist|
        MusicSetIteratorSequential.new(playlist)
      end

      @midi.track_iterator = MusicSetIteratorConcat.new(playlist_iterators + [MusicSetIteratorRandom.new(selective_collection)])
    else
      @midi.track_iterator = MusicSetIteratorRandom.new(selective_collection)
    end

    ##@sourceA.open(Path.new("c:/data/mp3/Shooby/shooby14.mp3"))
    @sourceA.open(Track.new(appPath() + Path.new("Test Sounds/click.mp3")))
    #@sourceB.open(Track.new(baseMCPath + "music/mp3/Alec Empire/The Destroyer/01. Hard Like It's a Pose.mp3"))
    @sourceB.open(Track.new(baseMCPath + Path.new("music/flac/20 Hardcore Classics/01. Square Dimensione - A Brand New Dance.flac")))
  
    #@sourceA.play
    #@sourceB.play
	end
	
	def run
    raise "--server must be given." if $argv_opts[:server].nil?
    uri = URI.parse($argv_opts[:server])

    $dj_server = DJAppServer.new($djApp, uri.scheme + "://" + uri.host, uri.port)
    $dj_server.start

		tempSetup

    PlatformOS.run_ruby_cmd_window('djgui.rb', 'Debug', $dj_server.url) if @run_gui
        
		@mainLoop.run
    @mainLoop.join
	end
	
	def do_eval(str)
	  eval(str)
	end

  def sources
    [@sourceA, @sourceB]
  end

  def focused_source
    if @crossfade.value < 0.4
      @sourceA
    elsif @crossfade.value > 0.6
      @sourceB
    else
      nil
    end
  end
	
  def update_freq
    UPDATEFREQ
  end
  
  def loaded
    super
    
    @volume_normalization_default = Dsound::Gain.new(@config_volume_normalization_default)
  end
  
  def on_song_transition
    @sourceA.finalize_song_play_event
    @sourceB.finalize_song_play_event
    
    if $use_event_db
      @last_song_transition_event = $djApp.event_database_session.new_event do 
        SongTransitionEvent.new(
          @last_song_transition_event,
          [@sourceA.song_play_event, @sourceB.song_play_event]
        )
      end
    end
  end
end

class DJAppServer
  include ModuleLogging
  
  def initialize(dj_app, local_address=nil, local_port=41069)
    if !local_address
      @local_address = "druby://"
      @public_local_address = "druby://localhost"
    else
      @local_address = local_address
      @public_local_address = local_address
    end
    
    @local_port = local_port
    @dj_app = dj_app
  end
  
  def start
    log "Starting communications at #{_url()}."
    options = {}
    options[:verbose] = true
    #options[:load_limit] = 512 * (1024 ** 2)
    DRb::DRbServer.new(_url(), @dj_app, options)
    log "DRb connection established at #{DRb.current_server.uri}."
  end
  
  def url
    "#{@public_local_address}:#{@local_port}"
  end

  def _url
    "#{@local_address}:#{@local_port}"
  end
end
