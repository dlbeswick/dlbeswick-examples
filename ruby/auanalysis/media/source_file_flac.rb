require 'media/source_file'
require 'transform/metadata'

class SourceFileFlac < SourceFile
  def self.extension
    'flac'
  end

  # tbd: move this to a 'sourcefileduplicating' superclass 
  def extract_raw_sound_data(source_track)
    [source_track.source_file.path]
  end
  
  def extract_metadata(source_tracks)
    # There can only be one source track per flac file
    source_track = source_tracks.first
    
    metaflac = Metaflac.new
    metaflac.output_metadata = true
    ProgressStdout.always_report = true
    result = metaflac.execute([path()], nil)
    
    if !result.empty?
      metadata = {}
        
      result.each_line do |line|
        match = /(.+?)=(.+?)$/.match(line)
        if match && match.length == 3
          metadata[match[1].downcase] = match[2].chomp
        end
      end
      
      if !metadata['artist'].nil?
        source_track.track.artist = source_album().library.artists.add_or_retrieve(metadata['artist'])
      end
      
      source_track.track.order = metadata['tracknumber']
      source_track.track.title = metadata['title']
      source_track.album_description = metadata['album']
      source_track.album_release_date = metadata['date']
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
    false
  end
  
  def sound_file
    raise "Invalid, fix this."
  end
end
