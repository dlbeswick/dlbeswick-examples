require 'GUIwx'
require 'drb'
require 'mutex_m'
require 'platform_os'
require 'std/SafeThread'
require 'view/View'

#ViewBase.log=true
#ViewAllAttributesCommon.log=true

$wx_mutex = Mutex.new

class TestApp < Wx::App
  def on_init
    window = Wx::Frame.new(
      nil, 
      -1, 
      "Test", 
      Wx::Point.new(-1, -1), 
      Wx::Size.new(
        400, 300
      )#,
      #Wx::TRANSPARENT_WINDOW
    )
    
    window.show
    
    GUI.app = self
    
    #test_drb(window)
    #test_gui_thread_guard(window)
    #test_thread(window)
    #test_thread_gc(window)
    #test_thread_pass(window, false)
    #test_mouse_cpu(window)
    puts 'init'
    test_destroy(window)
    
    true
  end

  class TestDestroyClass
    include Viewable
    attr_accessor :poo
    attr_accessor :wee
    attr_accessor :fart
    attr_accessor :mr_poo
    
    def initialize(mr_poo = nil)
      @poo = 'Poos'
      @wee = 1024
      @fart = [5,6,7,8,'cocks',(1..10)]
      @mr_poo = mr_poo 
    end
  end
  
  def test_destroy(window)
    views = []
      
    GUI.app.gc_stress(1)
    puts 'creating'
    
    window.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
    
    Wx::Timer.every(1) do
      puts 'start'
      it_texts = views.dup
      i = 0
      it_texts.each do |w|
        puts 'destroying'
        w.destroy
      end
      it_texts.clear
      views.clear
      puts 'make'
      views << View.new_for(TestDestroyClass.new, window)
      window.sizer().add(views.last)
      window.layout
      puts 'make'
      views << View.new_for(TestDestroyClass.new(TestDestroyClass.new(TestDestroyClass.new)), window)
      window.sizer().add(views.last)
      window.layout
      puts i
      puts views.length
      i += 1
    end

    puts 'done'
  end
  
  def test_drb(window)
    $queue = []
    $queue.extend(Mutex_m)
    
    Wx::Timer.every(15) do
      puts 'timer'
      
      queue = $queue.synchronize do
        $queue.dup
      end
      
      queue.each do |p|
        puts 'queue call'
        p.call
      end
    end
    
    $queue << proc do
      server_conn = DRbObject.new(nil, "druby://localhost:1410")
      DRb.start_service
      data = server_conn.data
      data.each_index do |i|
        puts i
        $queue.synchronize do
          $queue << proc {
            puts 'proc exec'
            Wx::StaticText.new(window, -1, data[i].to_s)
          }
        end 
      end
    end
  end
  
  def test_thread_pass(window, pass)
    if pass
      Wx::Timer.every(15) do
        Thread.pass
      end
    end
    
    SafeThread.new do
      loop do
        puts 'yyy'
        sleep 1
      end
    end
  end

  def test_gui_thread_guard(window)
    text_thread = Wx::StaticText.new(window, -1, '0')
    text_timer = Wx::StaticText.new(window, -1, '0')
    text_timer.move(100, 0)
    text_thread_2 = Wx::StaticText.new(window, -1, '0')
    text_thread_2.move(200, 0)
    
    $thread_val = 0 
    
    Wx::Timer.every(15) do
      Thread.pass
    end
    
    SafeThread.new do
      loop do
        $wx_mutex.synchronize do
          text_thread_2.set_label('pee')
        end
        #text_thread_2.set_label((text_thread.label.to_i + 1).to_s)
        $thread_val += 1 
      end
    end
    
    #Wx::Timer.every(500) do
    #  $wx_mutex.synchronize do
    #    text_timer.set_label((text_timer.label.to_i + 1).to_s)
    #    text_thread.label = $thread_val.to_s
    #  end
    #end
  end
  
  def test_thread(window)
    text_thread = Wx::StaticText.new(window, -1, '0')
    text_timer = Wx::StaticText.new(window, -1, '0')
    text_timer.move(100, 0)
    text_thread_2 = Wx::StaticText.new(window, -1, '0')
    text_thread_2.move(200, 0)
    
    $thread_val = 0 
    
    SafeThread.new do
      loop do
        $thread_val += 1 
      end
    end
    
    10.times do
      SafeThread.new do
        loop do
          100000.times do
            i = 1
          end 
        end
      end
    end
    
    Wx::Timer.every(500) do
      text_timer.set_label((text_timer.label.to_i + 1).to_s)
      text_thread.label = $thread_val.to_s
    end
  end

  def test_mouse_cpu(window)
    1000.times do
      Wx::StaticText.new(window, -1, '0')
    end
    Wx::UpdateUIEvent.set_update_interval(1000)
    #Wx::UpdateUIEvent.set_mode(UPDATE_UI_PROCESS_SPECIFIED)
  end
  
  def test_thread_gc(window)
    text_thread = Wx::StaticText.new(window, -1, '0')
    text_timer = Wx::StaticText.new(window, -1, '0')
    text_timer.move(100, 0)
    text_thread_2 = Wx::StaticText.new(window, -1, '0')
    text_thread_2.move(200, 0)
    
    $thread_val = 0 
    
    SafeThread.new do
      loop do
        $thread_val += 1 
      end
    end
    
    10.times do
      SafeThread.new do
        loop do
          100000.times do
            i = 1
          end 
        end
      end
    end
    
    GC.disable
    combo = Wx::ComboBox.new(window, -1, :style => Wx::CB_DROPDOWN | Wx::CB_READONLY)
    combo.append("text")
    combo.append("text")
    combo.append("text")
    combo.append("text")
    combo.append("text")
    
    Wx::Timer.every(500) do
      GC.enable
      text_timer.set_label((text_timer.label.to_i + 1).to_s)
      text_thread.label = $thread_val.to_s
      GC.disable
    end
  end
end

def thread_guard_test
  #module Test
  #  class X
  #    def poo
  #      puts 'hi!'
  #    end
  #  end
  #end
  #GUIWx.thread_guard(Test)
  #X.new.poo
end

#GUIWx.thread_guard
#GC.disable
TestApp.new.main_loop

# timer hack is not necessary for ruby 1.9. Actually, it is. I was just using a wx timer before.
# calling wx from any thread other than the main thread will cause a deadlock