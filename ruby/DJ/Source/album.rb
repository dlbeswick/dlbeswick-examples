require 'drb'

class Album
	include Comparable
  include DRb::DRbUndumped
	
	attr_reader :tracks
	attr_reader :title
	attr_writer :va

	def initialize(title)
		@title = title
		@tracks = []
		@va = nil
	end

	def addTrack(track)
		tracks << track
	end
	
	def va?
		return @va if @va

		if tracks.length == 1
			@va = false
			return @va
		end

		(1...tracks.length).each do |i|
			if tracks[i].artist != tracks[i-1].artist
				@va = true
				return @va
			end
		end

		@va = false
		return @va
	end
	
	def <=>(rhs)
		return @title.downcase <=> rhs.title.downcase
	end
end