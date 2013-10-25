require 'transform/export/task_by_program_instance'
require 'std/module_logging'

class TaskAlbumBoundaryAnalysis < TaskRunnerTask
  include ModuleLogging

  def initialize(sound_file_analyzer_tuples)
    super()
    
    @tasks = []
    sound_file_analyzer_tuples.each do |tuple|
      @tasks << TaskByProgramInstance.new(:TaskTrackBoundaryAnalysis, *tuple)
      depend_on(@tasks.last)
    end
  end

  def result
    @tasks.collect { |task| task.result }
  end

  def describe
    "#{super} [#{@sound_file.to_s}]"
  end
end
