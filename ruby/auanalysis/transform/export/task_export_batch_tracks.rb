require 'transform/export/transform_task'

class TaskExportBatchTracks < TransformTask
  def initialize(exporter, source_tracks)
    super()
    
    source_albums_to_source_tracks = {}
    source_tracks.each do |source_track|
      source_albums_to_source_tracks[source_track.source_album] ||= []
      source_albums_to_source_tracks[source_track.source_album] << source_track
    end
    
    source_albums_to_source_tracks.each do |source_album, source_tracks|
      depend_on TaskExportAlbum.new(exporter, source_album, source_tracks)
    end  
  end
end
