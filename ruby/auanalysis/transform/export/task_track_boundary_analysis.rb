require './media/source_file'
require './transform/export/transform_task'
require './track_boundary_analysis'
require 'media/source_album'
require 'media/library'

require 'std/EnumerableSubclasses'
require 'std/module_logging'

EnumerableSubclasses.require('sound_file_*')

class TaskTrackBoundaryAnalysis < TaskRunnerTask
  include ModuleLogging

  def initialize(analyzer, source_file)
    super()
    
    @description = source_file.to_s
    
    define_execution do
      log { "Analyzing #{source_file.sound_file.path}" }
      analyzer.calc(source_file.sound_file)
      analyzer.boundaries
    end
  end
  
  def describe
    "#{super} [#{@description}]"
  end
end
