require 'media/album'
require 'std/abstract'
require 'id3lib'
require 'std/raise'
require 'std/ConfigYAML'
require 'std/UniqueIDConfig'

class Track
  include ConfigYAML
  include Raise
  include UniqueIDConfig
  include Viewable
  
  persistent_reader :album

  persistent_infer :artist do
    if @album
      @album.artist
    else
      raise "No artist or album is defined for track '#{self}'."
    end
  end
  
  persistent_infer :release_date do
    if album()
      album().instance_variable_get(:@release_date)
    else
      nil
    end
  end
  
  persistent_infer :title do nil; end
  attr_model_value :title, ''
  
  # A string representing the order of the track in the album.
  # Commonly a single number, but can also be expressed as "1/12" or "1 of 12"
  persistent_infer :order do
    album().tracks().index(self) + 1
  end
  
  default_choices :artist do
    if album()
      album().library().artists.to_a.sort
    else
      []
    end
  end
  
  # hack for array view: tbd: fix so that 'add' param is not required
  def initialize(album = nil, add = true)
    if album && add
      album.add_track(self)
    else
      @album = album
    end
    symboldefaults_init(Track)
  end
  
  # Attempts to construct a track from the metadata in the media file at the given path.
  def self.new_from_path(album, path)
    if !album.library
      raise "Supplied album must have a library defined."
    end
    
    if path.extension == 'mp3'
      tag = ID3Lib::Tag.new(path.to_sys)
      
      result = Track.new(album)
      result.artist = album.library.artists.add_or_retrieve(tag.artist) if tag.artist
      result.title = tag.title if tag.title
      result.order = tag.track if tag.track
      result.release_date = tag.year if tag.year
      result
    else
      # tbd: make a 'media' base class that all types of media derive from.
      # then, modify the source file classes to make use of them.
      raise "Unsupported file type, please code support: #{path}"
    end
  end
  
  def library
    raise "No album defined." if !album()
    album().library
  end
  
  # tbd: make a data type that can return either the string representation or an inferred date object
  def release_date_as_date
    return nil if !@release_date
    
    if release_date().kind_of?(Date)
      @release_date
    else
      begin
        Date.strptime(@release_date.to_s, '%Y-%m-%d')
      rescue ArgumentError
        begin
          Date.strptime(@release_date.to_s, '%Y')
        rescue ArgumentError
          raise "Could not extract release date from the release date string '#{release_date()}'."
        end
      end
    end
  end
  
  # tbd: consolidate this code duplicated from Album 
  def release_year
    if release_date().kind_of?(Date)
      release_date().year
    else
      match = /(\d{4,4})/.match(release_date())
      raise "Could not extract release year from the release date string '#{release_date()}'." if !match 
        
      match.to_s
    end
  end
  
  # Overwrite data in the track with information from the given track.
  def set_from(track)
    if track._artist
      @artist = library().artists.add_or_retrieve(track._artist)
    else
      @artist = nil
    end
    
    @title = track.title
    @order = track._order
    @release_date = track.instance_variable_get(:@release_date)
  end
  
  def title
    if @title
      @title.dup
    else
      "Untitled"
    end
  end

  def source_track(album)
    album.source_tracks.find { |source_track| source_track.track == self }
  end
  
  def to_s
    title()
  end
  
  def _album=(album)
    @album = album
  end
end

