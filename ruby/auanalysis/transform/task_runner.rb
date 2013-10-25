require 'std/module_logging'
require 'std/SafeThread'

class TaskRunnerStackBased
  include ModuleLogging

  def initialize
    @tasks_and_dependants = {}
  end

  def record_dependant_for_task(task, dependant)
    dependants = @tasks_and_dependants[task] ||= []
    dependants << dependant if dependant
  end

  def tasks_without_dependants
    result = []

    @tasks_and_dependants.each do |task, dependants|
      if task.can_run? && !task.finished? && !dependants.any? { |dependant| dependant.can_run? && !dependant.finished? }
        result << task
      end
    end

    result
  end

  def run(task, maximum_parallel_tasks = 6, depth = 0, instigators = [])
    raise "Maximum number of parallel tasks must be positive." if maximum_parallel_tasks <= 0

    log { "#{' ' * depth}Task #{task} depends on:" }

    record_dependant_for_task(task, nil)

    instigators.each do |instigator|
      record_dependant_for_task(instigator, task)
    end

    task.dependencies.each do |task_dependency|
      run(task_dependency, maximum_parallel_tasks, depth + 1, instigators + [task])
    end

    if depth == 0
      start_time = Time.now

      loop do
        tasks_to_run = tasks_without_dependants()
        if tasks_to_run.empty?
          log { "Done. (#{format('%.2f', Time.now - start_time.to_f)} seconds)" }
          break
        end

        log { "-- Running parallel iteration --" }

        threads = []
        threads.extend(Mutex_m)

        tasks_to_run.each do |task|
          loop do
            thread_count = threads.synchronize { threads.length }
            break if thread_count < maximum_parallel_tasks
            sleep 1
          end

          block = proc do
            task.instance_eval do
              if can_run? && !finished?
                run_begin()
                run()
                run_end()
              end
              threads.synchronize { threads.delete(Thread.current) }
            end
          end

          threads << SafeThread.new(&block)
          threads.last.run
        end

        remaining_threads = threads.synchronize { threads.dup }
        remaining_threads.each { |thread| thread.join }
     end
    end
  end
end

class TaskRunnerSerial
  include ModuleLogging
  
  def initialize
    @active_tasks = []
    @active_tasks.extend(MonitorMixin)
    @active_tasks_list_free = @active_tasks.new_cond
    @log_m = Mutex.new
  end
  
  def run(task, maximum_parallel_tasks = 5, depth = 0, instigator = nil)
    need_wait = false
    task_description = ''
    
    task.synchronize do
      return if task.finished?
      
      task_description = if instigator
        "#{task.describe} for #{instigator.describe}."
      else
        task.describe
      end
    
      if task.running?
        need_wait = true
      else
        @log_m.synchronize { log { " " * depth * 2 + "Begin task #{task_description}." } }
        task.run_begin
      end
    end

    if need_wait
      @log_m.synchronize { log { " " * depth * 2 + "Waiting on #{task_description}." } }
      task.wait_until_complete()
      raise "Task done running, but apparently not finished." if !task.finished?
      return
    end
          
    dependency_threads = []  
    task.dependencies.each do |dependent_task| 
      thread = SafeThread.new do
        run(dependent_task, maximum_parallel_tasks, depth + 1, task)
      end
      
      dependency_threads << thread
      
      thread.run
    end
    dependency_threads.each { |thread| thread.join }
      
    if task.can_run?
      @active_tasks.synchronize do
        @active_tasks_list_free.wait_while { @active_tasks.length == maximum_parallel_tasks }
        @active_tasks << task
      end
        
      task.run
    end
    
    @log_m.synchronize { log { " " * depth * 2 + "Completed task #{task_description}." } }
      
    if task.can_run?
      @active_tasks.synchronize do
        @active_tasks.delete(task)
        @active_tasks.synchronize { @active_tasks_list_free.signal }
      end
    end
    
    task.run_end

    if depth == 0
      cleanup(task)
    end
  end

  def cleanup(task)
    task.dependencies.each do |dependent_task|
      cleanup(dependent_task)
    end
    
    if !task.cleaned?
      log { "Cleaning up task #{task}" }
      task.cleanup
    end
  end
end

TaskRunner = TaskRunnerStackBased
TaskRunner.log = true
