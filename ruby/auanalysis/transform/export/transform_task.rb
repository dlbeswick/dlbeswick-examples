require 'std/raise'
require 'mutex_m'

class TaskRunnerTask
  include Raise
  
  attr_reader :dependencies
  attr_reader :result
  
  def initialize
    extend(Mutex_m)
    @finished_mutex = Mutex.new
    @running_monitor = Monitor.new
    @running_cond = @running_monitor.new_cond 
    @dependencies = []
  end
  
  def describe
    self.class.name
  end
  
  def can_run?
    !@execution_block.nil?
  end
  
  def cleaned?
    @cleaned == true
  end
  
  def cleanup
    @cleaned = true
    _cleanup()
  end
  
  def finished?
    @finished_mutex.synchronize { @finished == true }
  end
  
  def maximum_parallel_tasks
    nil
  end
  
  def running?
    @running_monitor.synchronize { @running == true }
  end
  
  def depend_on(task_or_tasks)
    @dependencies += Array(task_or_tasks)
  end
  
  def define_execution(&block)
    @execution_block = block
  end
  
  def run_begin
    @running_monitor.synchronize do
      @finished_mutex.synchronize do
        raise "Task already running (#{describe()})." if @running
        @running = true
        raise "Task already finished (#{describe()})." if @finished
      end
    end
  end
  
  def run
    @result = @execution_block.call if @execution_block
  end
  
  def run_end
    @finished_mutex.synchronize do
      @running_monitor.synchronize do
        @finished = true
        @running = false
        @running_cond.signal
      end
    end
  end
  
  def wait_until_complete
    while running?
      sleep 1
    end
    #@running_monitor.synchronize do 
    #  @running_cond.wait_while { @running == true } if @running
    #end
  end
  
  def to_s
    describe()
  end
  
protected
  def _cleanup()
  end
end

class TransformTask < TaskRunnerTask
end
