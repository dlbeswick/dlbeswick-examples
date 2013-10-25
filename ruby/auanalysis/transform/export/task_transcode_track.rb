require 'transform/export/task_source_track'

class TaskTranscodeTrack < TaskSourceTrack
  def initialize(source_track, format, extract_raw_sound_data_task)
    super(source_track)
    
    @format = format
    
    define_execution do
      output_path = source_track.dest_path(format)
      format.export_track(output_path, extract_raw_sound_data_task.paths, source_track)
    end
  end
  
  def describe
    "#{super} [#{@format.to_s}]"
  end
end