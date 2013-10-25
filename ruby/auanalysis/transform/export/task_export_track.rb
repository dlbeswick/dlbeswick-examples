require 'transform/export/task_source_track'
require 'transform/export/task_transcode_track'
require 'transform/export/task_extract_raw_sound_data'

class TaskExportTrack < TaskSourceTrack
  def initialize(exporter, source_track)
    super(source_track)
    
    raw_sound_data_task = TaskExtractRawSoundData.new(exporter, source_track)
    
    exporter.output_formats.each do |format|
      if format.valid_for?(source_track)
        task = TaskTranscodeTrack.new(source_track, format, raw_sound_data_task)
        task.depend_on(raw_sound_data_task)
        depend_on(task)
      end
    end
  end
end