require 'std/ConfigYAML'
require './Source/ResourceFile'
require 'drb'

# Tracks are stored as resources to avoid multiple file accesses for their file information,
# so tracks are also shared between albums in music collections.
# They don't have a reference to their 'Album' object for this reason, and so Tracks should
# generally be enumerated in the context of a MusicCollection. Mappings from Tracks to Albums
# can be generated via a MusicCollection.
class Track < Resource::File
	include Comparable
  include DRb::DRbUndumped
	
	persistent :albumTitle
	persistent :artist
	persistent :title
  persistent :volume_normalization_db
	persistent :tagGenerated
	
	default :volume_normalization_db, nil
	
	def self.get(path)
		resource = super(path.to_s.downcase)
		
		if !resource
			resource = new(path) 
			cache(resource)
		end

		resource
	end
	
	def <=>(rhs)
		v = @albumTitle.downcase <=> rhs.albumTitle.downcase

		if v == 0
			return @title.downcase <=> rhs.title.downcase
		end
		
		v
	end
	
	def initialize(path)
		super
		
		@tagGenerated = false
		@artist = ''
		@title = ''
		@albumTitle = ''
		
		@path = path
		if !@path.empty?
			generateTagData
		end
	end

	def generateTagData
		if !@tagGenerated
			tag = nil
			
			begin
				tag = if @path.extension == 'mp3'
				  Mediatag::ID3.new(@path.to_sys)
				else
          Mediatag::MediaTagSox.new(@path.to_sys)
				end
			rescue
			end
			
			if tag	
				@artist = tag.artist
				@albumTitle = tag.album
				@title = tag.title
				
				if tag.hasVolumeNormalisationTrack
				  @volume_normalization_db = Dsound::Gain.new(tag.volumeNormalisationTrack)
        end
			end
	
			@artist = @path.parent.parent.last.to_s if !@artist || @artist.empty?
			@albumTitle = @path.parent.last.to_s if !@albumTitle || @albumTitle.empty?
			@title = @path.file.to_s if !@title || @title.empty?

			@tagGenerated = true
		end
	end

	def albumTitle
		@albumTitle
	end
	
	def artist
		@artist
	end
	
  def volume_normalization_db?
    !@volume_normalization_db.nil?
  end
  
	# Returns null if no data is present
	def volume_normalization_db
    @volume_normalization_db
	end

	def description
		return self.title
	end
	
	def title
		return @title
	end
end
