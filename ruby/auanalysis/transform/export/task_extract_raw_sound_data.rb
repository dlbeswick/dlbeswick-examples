require 'transform/export/task_source_track'

class TaskExtractRawSoundData < TaskSourceTrack
  attr_accessor :paths
  
  def initialize(exporter, source_track)
    super(source_track)

    define_execution do
      result = exporter.extract_raw_sound_data(source_track)
      
      # standardise the output of "extract_raw_sound_data"
      @paths = []
      @temp_files = []
      result.each do |file_or_temp_file|
        if file_or_temp_file.respond_to?(:path)
          @temp_files << file_or_temp_file
        else
          @paths << file_or_temp_file
        end
      end
    end
  end

  def paths
    @paths + @temp_files.collect { |tf| tf.path }
  end
    
  def _cleanup
    @temp_files.each { |tf| tf.close! }
  end
end