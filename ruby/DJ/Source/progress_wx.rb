require 'std/progress'
require 'std/EnumerableInstances'

class ProgressStdoutTimerWx < ProgressStdout
  include EnumerableInstances
  
  def self.request_exit
    self.each_instance do |instance|
      log { "Asking wx progress timer '#{instance.description}' to exit." }
      instance.request_exit
    end
  end
  
  def request_exit
    @exit_requested = true
  end
  
  def next(time_delay)
    @is_block_execution_finished = false
    @next_time_delay = time_delay
  end
  
protected
  def yield_to_block(&block)
    timer = Wx::Timer.after(1) do
      if !@exit_requested
        @is_block_execution_finished = true
        @timer = timer
        yield self
        
        if !@is_block_execution_finished
          @timer.start(@next_time_delay, true)
        else
          cleanup()
        end
      else
        log { "Wx progress timer '#{description()}' exited on request." }
        cleanup()
      end 
    end
  end
end