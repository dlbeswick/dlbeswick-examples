require 'std/platform_gui'
require 'std/platform_os'
require 'std/module_logging'
require 'std/raise'
require 'wx'

PlatformGUI.platform = 'Wx' 

# fix common name collisions
class Wx::Window
	alias_method :wx_disconnect, :disconnect
	alias_method :wx_raise, :raise
	
	def raise(e)
		Kernel.raise(e)
	end
end

module GUIWx
  include ModuleLogging
  include Raise
  
  def self.thread_guard(guard_module = Wx)
    $guiwx_main_thread = Thread.current
     
    guard_module.constants.each do |constant|
      constant = guard_module.const_get(constant)
      if constant.kind_of?(Class)
        constant.instance_methods(false).each do |symbol|
          next if not /\w/ =~ symbol
          next if symbol == :to_s
          
          note { "Guarding #{constant}::#{symbol}..." }
          begin
            constant.module_eval <<-EOS 
              alias_method :thread_guard_#{symbol}, :#{symbol}
            
              def #{symbol}(*args,&block)
                if Thread.current != $guiwx_main_thread
                  error_message = "Wx call outside of main thread." 
                  $stderr.puts error_message
                  $stderr.puts caller()
                  exit()
                end
  
                send(:thread_guard_#{symbol},*args,&block)
              end
            EOS
          rescue Exception=>e
            puts e
          end
        end

        constant.instance_eval do        
          alias_method :thread_guard_initialize, :initialize
          
          def initialize(*args)
            if Thread.current != $guiwx_main_thread
              $stderr.puts "Wx call outside of main thread."
              $stderr.puts caller()
              exit
            end
  
            send(:thread_guard_initialize, *args)
          end
        end
      end
    end
  end

  def window_stats
    windows = {}
    ObjectSpace.each_object do |obj|
      if obj.kind_of?(Wx::Window)
        windows[obj.class] ||= 0
        windows[obj.class] += 1
      end 
    end
    
    window_results = []
    windows.each do |k, v|
      window_results << [k, v]
    end
    
    window_results = window_results.sort_by { |e| e.last } 
    
    window_results.each do |e|
      puts "#{e.first}: #{e.last}"
    end
    
    puts "#{window_results.length} windows."
  end
end

module GUI
  def self.app
    @app
  end
  
  def self.app=(app)
    @app = app
  end
  
  def self.destroy_window(window)
    window.hide
    #window.reparent(nil)
    #@app.destroy_window(window, true)
  end

  # This code should ensure that any GUI calls occur in the main thread. If an application creates
  # situations where the below code no longer satisfies that requirement, then it should re-implement
  # this method.
  if !respond_to?(:ensure_main_thread)
    def self.ensure_main_thread(&block)
      block.call
    end
  end
  
  def self.main_window=(window)
    @@main_window = window
  end
  
  def self.main_window
    @@main_window
  end

  def self.user_get_dir(parent, title="Select a folder.", start_path=Path.new)
    dlg = Wx::DirDialog.new(parent, title, start_path.to_sys)
    
    if dlg.show_modal() == Wx::ID_OK
      Path.to_directory(dlg.get_path)
    else
      nil
    end
  end
  
  def self.user_get_file(parent, title="Select a file.", start_path=Path.new)
    dlg = Wx::FileDialog.new(parent, title, start_path.directory.to_sys, start_path.file.to_sys)
    
    if dlg.show_modal() == Wx::ID_OK
      Path.new(dlg.get_path)
    else
      nil
    end
  end
  
  class WxApp < Wx::App
    def initialize(block)
      super()
      @block = block
    end
    
    def on_init
      main_window = Wx::Frame.new(nil, -1, $0)
      
      main_window.evt_close do
        main_window.destroy
      end
      
      GUI.main_window = main_window
      
      @block.call(main_window)
      main_window.show
    end
  end
    
  def self.start(&block)
    begin
      @app = WxApp.new(block)
      @app.main_loop
    rescue Exception=>e
      $stderr.puts e
      $stderr.puts e.backtrace
      Kernel.exit
    end
  end
end

class PlatformGUI
  def self.gtk?
    # tbd: replace this with a proper check.
    # wx-config --basename is probably the best option
    PlatformOS.linux?
  end
end
