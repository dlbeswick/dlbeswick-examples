require 'transform/export/task_source_album'

class TaskPostExportAlbum < TaskSourceAlbum
  def initialize(source_album, output_format, outputs)
    super(source_album)

    @format = output_format
       
    define_execution do
      output_paths = outputs.collect { |o| o.dest_path }
      output_format.post_export_album(output_paths, source_album)

      # touch all output files with the same date, so they sort by creation date properly.
      touch_time = Time.now
      File.utime(touch_time, touch_time, *output_paths.collect{ |p| p.to_sys })
    end
  end
  
  def describe
    "#{super} [#{@format.to_s}]"
  end
end
