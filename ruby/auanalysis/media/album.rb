require 'media/artist'
require 'media/track'
require 'media/modification_watcher'
require 'std/ConfigYAML'
require 'std/ObjectHost'
require 'std/UniqueIDConfig'

# A music album.
class Album
  include ConfigYAML
  include ModificationWatcher
  include ObjectHost
  include UniqueIDConfig
  include Viewable
  
  persistent_accessor :artist
  persistent_accessor :catalogue_id
  persistent_infer :format do library().formats['12" vinyl']; end
  persistent_accessor :media_quantity
  persistent_infer :title do "Untitled"; end
  persistent_reader :library
  
  # This field can be any string indicating date/time information, or a Date object.
  # Other accessors such as 'release_year' will attempt to format this string.  
  persistent_infer :release_date do 
    common_dates = tracks().collect{ |t| t.release_date_as_date }.uniq
    if common_dates.length == 1
      tracks().first.release_date
    else
      'Unknown'
    end   
  end
   
  attr_model_value :release_date, "1900-01-01"
  
  persistent_reader :tracks
  
  default :artist do Artist.various(library()); end
  default :catalogue_id, ''
  default :media_quantity, 1
  default :title, 'Untitled'

  default_noinit :tracks do [Track.new(self, false)]; end

  default_choices :artist do library().artists.to_a.sort; end
  default_choices :format do library().formats.to_a.sort; end
    
  host :tracks
  
  no_view :library 
  
  def initialize(library)
    @library = library
    @tracks = []
    symboldefaults_init(Album)
  end

  # hack for array view: remove 'add' param
  def add_track(track)
    raise "Track already belongs to album '#{track.album}'." if track.album
    @tracks << track if !@tracks.include?(track)
    track._album = self
  end
  
  def artist?
    @artist != nil
  end
  
  def clear_tracks
    @tracks.clear
  end
  
  # Sometimes the album artist can be better determined from the track metadata than the source file's
  # directory hierarchy. This method will return the album artist if possible.
  # This method will also clear artist information on each track if there is an album artist.
  def infer_album_artist_from_track_data
    if track_artists().length == 1
      self.artist = track_artists()[0]
      
      @tracks.each do |track|
        track.artist = nil
      end
    else
      raise "No album artist could be inferred."
    end
  end
  
  def is_va?
    @artist.equal?(Artist.various(library()))
  end
  
  def release_year
    if release_date().kind_of?(Time)
      release_date().year
    elsif release_date().kind_of?(NilClass)
      nil
    else
      match = /(\d{4,4})/.match(release_date())
      raise "Could not extract release year from the release date string '#{release_date()}'." if !match 
        
      match.to_s
    end
  end
  
  # Overwrite data in the album with information from the given album.
  def set_from(album)
    @artist = library.artists.add_or_retrieve(album.artist)
    @catalogue_id = album.catalogue_id.dup
    
    if album.format
      @format = library.formats.add_or_retrieve(album.format)
    else
      @format = library.formats['Unknown/Other']
    end
    
    @media_quantity = album.media_quantity 
    @title = album.title.dup
    @release_date = album.release_date
    
    clear_tracks()
    
    album.tracks.each do |track|
      new_track = Track.new(self)
      new_track.set_from(track)
    end
  end
  
  def set_tracks_from(album)
    clear_tracks()
    
    album.tracks.each do |track|
      new_track = Track.new(self)
      new_track.set_from(track)
    end
  end
  
  def sort_tracks
    @tracks.replace(@tracks.sort_by do |track|
      if track.order != nil 
        track.order.to_i
      else
        @tracks.index(track)
      end
    end)
  end

  def to_s
    "#{artist()} - #{title()}"
  end
  
  # Returns a copy of the tracks array.
  # tbd: find a way of preventing this array from being modified because members must have their album
  # set on add.
  # freeze can't currently be used because of instance variable setting by Viewable. 
  #def tracks
  #  @tracks.dup.freeze
  #end

  def tracks=(tracks)
    tracks.each do |track|
      raise "A track in the given array does not belong to this album." if !track.album.equal?(self)
    end
    
    @tracks = tracks
  end
  
  # Returns all artists referenced in this album: track artists and the album artist. 
  def all_artists
    (track_artists() + artist()).compact.uniq
  end
    
  # Returns the set of unique track artists in this album. 
  def track_artists
    @tracks.collect{ |t| t.artist }.compact.uniq
  end
  
  def untitled?
    @title == Album.untitled
  end

  def _library=(library)
    @library = library
  end
end

