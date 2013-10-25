require 'media/source_file'

class SourceFileWav < SourceFile
  def self.extension
    'wav'
  end

  def extract_raw_sound_data(source_track)
    [source_track.source_file.path]
  end
  
  def _extract_source_tracks
    [SourceTrack.new(source_album(), self, path().file.literal, Track.new(source_album()))]
  end

  def format_has_metadata?
    false
  end

  def lossy?
    false
  end
  
  def sound_file
    raise "Invalid, fix this."
  end
end
