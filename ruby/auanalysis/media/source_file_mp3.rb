require 'media/source_file'
require 'id3lib'

class SourceFileMP3 < SourceFile
  def self.extension
    'mp3'
  end

  # tbd: move this to a 'sourcefileduplicating' superclass 
  def extract_raw_sound_data(source_track)
    [source_track.source_file.path]
  end
  
  def extract_metadata(source_tracks)
    # There can only be one source track per flac file
    source_track = source_tracks.first
    
    tag = ID3Lib::Tag.new(path().to_sys)
    if !tag.empty?
      if source_album().artist && tag.artist.nil? == false && tag.artist != source_album().artist.name
        source_track.track.artist = source_album().library.artists.add_or_retrieve(tag.artist)
      end
      
      source_track.track.order = tag.track
      source_track.track.title = tag.title
      source_track.track.release_date = tag.year
    end
  end
  
  def format_has_metadata?
    true
  end

  def _extract_source_tracks
    source_track = SourceTrack.new(source_album(), self, path().file.literal, Track.new(source_album()))
    
    [source_track]
  end

  def lossy?
    true
  end
  
  def sound_file
    raise "Invalid, fix this."
  end
end
