require 'transform/export/transform_task'
require 'open3'
require 'std/module_logging'

# Runs another instance of the program to complete the given task.
class TaskByProgramInstance < TaskRunnerTask
  include ModuleLogging

  def initialize(task_class_or_class_symbol, *task_params)
    super()
    
    @describe = "exec #{task_class_or_class_symbol}(#{task_params.join(",")})"
    
    define_execution do
      params = ['ruby', "auanalysis.rb", "--task=#{task_class_or_class_symbol}"]
      log { "popen: #{params.join(" ")}" }

      params_marshalled = Marshal.dump(task_params)

      result = Open3.popen3(*params) do |stdin, stdout, stderr|
        begin
          stdin.write(params_marshalled)
          stdin.close
          output = stdout.read
          Marshal.load(output)
        rescue Exception => e
          log { "Failed: #{e}" }
          log { stderr.read }
          raise
        end
      end

      if result.first == false
        raise "Task failed: #{result[1..-1].join(',')}"
      else
        result.last
      end
    end
  end
  
  def describe
    "#{super} [#{@describe}]"
  end
end
