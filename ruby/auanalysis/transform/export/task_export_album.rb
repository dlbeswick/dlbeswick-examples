require 'transform/export/task_source_album'
require 'transform/export/task_export_track'
require 'transform/export/task_post_export_album'

class TaskExportAlbum < TaskSourceAlbum
  def initialize(exporter, source_album, source_tracks=source_album.source_tracks)
    super(source_album)
    
    if source_tracks.collect{ |t| t.source_album }.uniq.length != 1
      raise "All source tracks must be from the source album."
    end
      
    existing_outputs = exporter.existing_outputs(source_tracks)  
    raise "Output files already exist: #{existing_outputs.join($/)}" if !existing_outputs.empty?
      
    track_tasks = []
    source_album.source_tracks.each do |source_track|
      if source_tracks.include?(source_track)
        track_tasks << TaskExportTrack.new(exporter, source_track)
      end
    end
    depend_on(track_tasks)
    
    # Be sure to use all of the album tracks here, even if they weren't to be exported. 
    # This supports operations such as calculating album replaygain.
    exporter.output_formats_to_outputs(source_album.source_tracks).each do |output_format, outputs|
      task = TaskPostExportAlbum.new(source_album, output_format, outputs)
      task.depend_on(track_tasks)
      depend_on(task)
    end
    
    define_execution do
      source_album.source_tracks.each do |source_track|
        source_track.mark_processed
      end
    end
  end
end