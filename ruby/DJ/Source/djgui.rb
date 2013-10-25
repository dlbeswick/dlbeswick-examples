$: << Dir.pwd

require 'Source/djinit'
require 'std/module_logging'
include ModuleLogging

#async
#$:.insert(0, 'D:\depot\fork\wxruby-2.0.1\lib')
#require 'wx'

if $debug_ruby19_mingw
  require 'rubygems'
  $: << "."
  $: << "d:/depot/projects/ruby"
  $: << "d:/depot/projects/ruby/std"
  
  require 'std/platform_os'
end

module GUI
  def self.ensure_main_thread(&block)
    if Thread.current != $main_thread
      $djgui.queue(&block)
      nil
    else
      yield
    end
  end
end

$profile_gui = false
if $profile_gui
  require 'ruby-prof'
end

begin
  $stdout.sync = true
  $stderr.sync = true
  
  require 'std/GUIWx'
  require './Source/source'
  require 'std/view/View'
  require 'std/EnumerableSubclasses'
  
  EnumerableSubclasses.log = 3
  EnumerableSubclasses.require(Path.new('./Source/Views'))
    
  require 'std/platform_os'
  
  # common modules
  if PlatformOS.osx?
    require 'std/GUIWx'
  else
    require 'std/GUIWx'
    #require 'GUIfox'
  end
  
  $main_thread = Thread.current
  GUIWx.thread_guard
    
  #EnumerableInstances.debug=true
  require 'std/view/View'
  require 'std/view/Viewable'
  require 'std/view/all_attributes'
  require './Source/Views/MappingView'
  
  # view classes
  require './Source/DevContainer'
  require './Source/DNDArea'
  require './Source/MenuBar'
  require 'std/EvalWindow'
  require './Source/visualactuator'

  module DJGuiBase
    include ModuleLogging
    
    attr_reader :app
    attr_reader :dragAndDrop
    attr_reader :fontBold
    attr_reader :masterWindow
    attr_reader :menuBar
    
    def initialize_djapp(dj_app)
      @app = dj_app
      $djApp = dj_app
      DRb.remote_authority = $djApp
      
      if PlatformGUI.fox?
        @fxApp = FXApp.new
        @fxApp.create
        makeAppWindow
  
        @fontBold = FXFont.new(@fxApp, 'arial,100,bold')
        @fontBold = FXFont.new(@fxApp, 'arial,100,bold')
      end
    end
    
    def temp_setup()
      note { "Init master view." }
      # master view
      viewMaster = SourceViewContainers::Tabbed.new(app().master, masterWindow())

      note { "Init monitor view." }
    # monitor views
      viewMonitor = SourceViewContainers::Tabbed.new(app().monitor, masterWindow())
    
      note { "Init source view A." }
    # source views
      viewA = SourceViewContainers::Tabbed.new(app().sourceA, masterWindow())

      note { "Init source view B." }
      viewB = SourceViewContainers::Tabbed.new(app().sourceB, masterWindow())
    
      if PlatformGUI.fox?
        masterWindow().setFocus
      end
      
      note { "Init music collection view." }
    # music collection views
      ViewContainers::Tabbed.new(app().musicCollections, masterWindow(), MusicCollectionListView)
    
      note { "Init midi view." }
      ViewContainers::Tabbed.new(app().midi, masterWindow(), MIDIControllerView)
      
      note { "Init eval window." }
      EvalWindow.new(masterWindow())
    end
    
    def run
      temp_setup
      
      if PlatformGUI.fox?
        fxApp.run
      end
    end
    
    def screenWidth
      if PlatformGUI.fox?
        self.fxApp.rootWindow.width
      else
        Wx::SystemSettings::get_metric(Wx::SYS_SCREEN_X)
      end
    end
    
    def screenHeight
      if PlatformGUI.fox?
        self.fxApp.rootWindow.height
      else
        Wx::SystemSettings::get_metric(Wx::SYS_SCREEN_Y)
      end
    end
    
    def quit
      if PlatformGUI.fox?
        self.fxApp.stop
      else
        @masterWindow.close
      end
    end
      
  :protected
    if PlatformGUI.fox?
      def makeAppWindow
        style = DECOR_NONE
        
        @masterWindow = FXMainWindow.new(self.fxApp, 'DJ', nil, nil, style, 0, 0, self.screenWidth, self.screenHeight)
        @masterWindow.create
        @masterWindow.show
    
        @masterWindow.connect(SEL_CLOSE) {
          quit
        }
        
        onMasterWindowCreated
        
        makeMenuBar
      end
    end
    
    def onMasterWindowCreated
      @dragAndDrop = DragAndDrop.new(@masterWindow)
      
      if PlatformGUI.fox?
        app().audioLibrary().master_window = @masterWindow.xid
      else
        app().audioLibrary().master_window = @masterWindow.handle
      end
    end
    
    def makeMenuBar
      @menuBar = MenuBar.new(@masterWindow)
      
      MenuBarCategory.new(@menuBar, 'DJ') do |cat|
        MenuBarItem.new(cat, 'Save') do
          save
        end
    
        MenuBarItem.new(cat, 'Exit') do
          quit
        end
    
        cat.separate
    
        MenuBarItem.new(cat, 'Dev') do
          self.dev = !self.dev
        end
      end
    end
  end
  
  if PlatformGUI.fox?
    class DJGui
      include ConfigYAML
      include DJAppBase
      
      def initialize
        initialize_djapp
        
        DNDArea.app = self
      end
    end
  else
    class DJGui < Wx::App
      include ConfigYAML
      include DJGuiBase
      
      attr_reader :closing
  
      def initialize(dj_app)
        super()
        initialize_djapp(dj_app)
        @gui_queue = []
        @gui_queue.extend(Mutex_m)
      end
  
      def close
        return if @closing
        @closing = true

        $djgui.timers_stop()
        
        log { "Requesting app exit..." }
        @app.close
        (1...10).each do
          begin
            @app.hello
          rescue
            log { "App seems to have exited." }
            $app_pid = nil
            break
          end
          sleep 1
        end
        
        @dragAndDrop.cleanup if @dragAndDrop
        
        Progress.run("Asking wx progress timer loops to exit...") do
          ProgressStdoutTimerWx.request_exit
          
          loop do
            sleep(0.1)
            
            ProgressStdoutTimerWx.instances.each do |timer|
              if !timer.finished?
                next
              end
            end
            
            break
          end
        end
        
        if @masterWindow
          @masterWindow.destroy_children
          @masterWindow.destroy
          @masterWindow = nil
        end
      end
      
      def on_init
        $djgui = self 
        
        @masterWindow = Wx::Frame.new(
          nil, 
          -1, 
          "DJ", 
          Wx::Point.new(-1, -1), 
          Wx::Size.new(
            self.screenWidth, 25
          )#,
          #Wx::TRANSPARENT_WINDOW
        )
        
        #@masterWindow.evt_erase_background do
        #end
        
        #def @masterWindow.has_transparent_background
        # true
        #end
        
        @masterWindow.evt_close do
          close
        end
        
        @masterWindow.evt_menu(Wx::ID_EXIT) do
          close
          false
        end
  
        self.top_window = @masterWindow
        # tbd: 
        @masterWindow.show if !PlatformOS.gui_has_menu_bar_per_window?
        
        if true
          #Thread.current.priority = -100
          thread_schedule          
        end
        
        onMasterWindowCreated
  
        makeMenuBar
        @masterWindow.fit
      
        #GC.disable if PlatformRuby.v19?

        #GUIWx.visible_garbage=true
        
        Wx::IdleEvent.set_mode(Wx::IDLE_PROCESS_SPECIFIED) if PlatformOS.windows?
        #Wx::UpdateUIEvent.set_update_interval(-1)
        Wx::UpdateUIEvent.set_mode(Wx::UPDATE_UI_PROCESS_SPECIFIED) if PlatformOS.windows?
        
        sourcemodifier_bulk_notify_listen()
        
        temp_setup

        #GUIWx.window_stats
        
        RubyProf.start if $profile_gui
        
        true
      end
      
      def queue(&proc_obj)
        #xx 'queue'
        @gui_queue.synchronize do
          #xx 'synced'
          #xx 'caller'
          #xx '---------------------'
          #puts caller()
          #xx '---------------------'
          #@gui_queue << [proc_obj, caller()]
          @gui_queue << [proc_obj, nil]
        end
        #xx 'end queue'
      end
      
      def sourcemodifier_bulk_notify_listen
        @sourcemodifier_bulk_notify_listen_proc = proc do |updates|
          raise "Update info is a drbobject but should be an array, did dumping to an array fail?" if updates.kind_of?(DRb::DRbObject) 
        
          updates.each do |element|
            actuator = element.first
            new_value = element.last
            
            if actuator.kind_of?(DRb::DRbObject)
              err = "Actuator reference is a drbobject but should have been converted back to a regular object reference on return. This can be caused by instantiating a local drbobject with a different uri to that of the local server. Detail: #{actuator.viewable_class}, uri:#{actuator.__drburi}, ref:#{actuator.__drbref}, here:#{DRb.current_server.uri}"
              raise err
            end
            
            queue { actuator.updateFromSource(new_value) }
          end
          
          nil
        end
        
        app().sourcemodifier_bulk_notifier.notify_listen(SourceModifierBulkNotifier::Notify, self, &@sourcemodifier_bulk_notify_listen_proc)
      end
      
      def thread_schedule
        work_queue = []
          
        Wx::Timer.every(100) do
          #GC.disable if PlatformRuby.v19?
          
          if @gui_queue && !@gui_queue.empty?
            @gui_queue.synchronize do
              work_queue.replace(@gui_queue)
              @gui_queue.clear
            end
           
            work_queue.each_with_index do |queue_entry, i|
              queue_entry.first.call
            end
          end
          
          Thread.pass
        end
      end
      
      def timer(after, &block)
        @timers ||= []
        
        @timers.delete_if do |timer|
          !timer.running?
        end
        
        new_timer = Wx::Timer.after(after) do yield new_timer; end
        @timers << new_timer
        new_timer
      end
      
      def timers_stop
        if @timers
          @timers.each do |timer|
            log { "Stopping timer #{timer}..." }
            timer.stop
          end
          log { "Timers stopped." }
        end
      end
      
      def timer_on_finished(timer)
        @timers.delete(timer)
      end
      
      def on_unhandled_exception
        raise "Wx exception." 
      end
      
      def on_exception_in_main_loop
        raise "Wx main loop exception." 
      end
      
      def profile
        app().profile
        
        SafeThread.new do
          RubyProf.start
          sleep 15
          result = RubyProf.stop
          puts "Stopped profiling."
          printer = RubyProf::GraphHtmlPrinter.new(result)
          
          printer.print(File.new("profile_gui.html", "w"), :min_percent=>2)
          puts "Wrote profiler data to 'profile_gui.html'"
        end
      end
  
      def run
        #async
        #main_loop_async
        if $profile_gui
          #RubyProf.start
        end
        
        main_loop
        
        if $profile_gui
          result = RubyProf.stop
          puts "Stopped profiling."
          printer = RubyProf::GraphHtmlPrinter.new(result)
          
          printer.print(File.new("profiler.html", "w"), :min_percent=>2)
          puts "Wrote profiler data to 'profiler.html'"
        end
      end
      
      def test_app(str)
        app().do_eval(str)
      end
    end
  end

  DJGui.log = true
  #ViewBase.log = true
  #ViewBase.log_time = true
  #Viewable.log=true
  #ViewAllAttributesCommon.log=true
  
  target_address = $argv_opts[:server]
  raise "The GUI module must be given a drb server address." if target_address.nil?
  
  log { "Attempting to connect to drb address '#{target_address}'" }
    
  match = /(druby:\/\/.+?\:)(.*)/.match(target_address)
  raise "No port given or bad address." if !match
  
  base = match[1]
  port = match[2].to_i
  
  DRb.start_service
  server = nil
  
  begin
    server = DRbObject.new(nil, "#{base}#{port}")
    server.hello
    log { "Existing DJ server active, connected." }
  rescue DRb::DRbConnError=>e
    should_spawn_server = $argv_opts[:run_server] == true
    
    if should_spawn_server
      server = nil
      $persist_app = false
      log { "Connection unsuccessful, attempting to spawn server." }

      if !$persist_app 
        PlatformOS.run_ruby_cmd_window('Source/dj.rb', $argv_orig)
      else
        PlatformOS.run_ruby_cmd_window_persist('Source/dj.rb', $argv_orig)
      end
    end
  rescue Exception=>e
    $stderr.puts "An non-network-related exception occurred while trying to spawn the server, please review."
    $stderr.puts e.class
    $stderr.puts e
    $stderr.puts e.backtrace
    exit
  end
  
  if !server
    15.times do
      begin
        puts "Attempting connection to DJ server..."
        server = DRbObject.new(nil, "#{base}#{port}")
        server.hello
        break
      rescue
        sleep 1
      end
    end
    server.hello
    $app_pid = server.pid
    puts "Connected. (Server pid is #{$app_pid})"
  end

  DJGui.new(server).run
  #async
  #while !$djgui.closing
  #  sleep 1
  #end

  begin
    log { "Requesting app exit..." }
    server.close
    (1...10).each do
      begin
        server.hello
      rescue
        log { "App seems to have exited." }
        $app_pid = nil
        break
      end
      sleep 1
    end
  rescue
  end
  
rescue Exception=>e
  $stderr.puts e.class
  $stderr.puts e.backtrace
  $stderr.puts
  $stderr.puts e
ensure
  if !$persist_app && $app_pid
    begin
      puts "Killing dj app: #{$app_pid}"
      Process.kill("KILL", $app_pid)
      puts "Killed."
    rescue Exception=>e
      puts "Exception while killing app:"
      puts e
    end
  end 
end
