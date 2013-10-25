require './Source/album'
require './Source/track'
require 'std/notify'
require 'std/ConfigYAML'
require 'std/module_logging'
require 'std/view/Viewable'

class MusicCollectionList < Array
  def viewable_dirtyTestReference
    length()
  end
end

class MusicCollection
  include ModuleLogging
	include NotifyHost
	include Viewable
	
	class NotifyBuildStatus < Notify::Base
		def onStatusChange(building)
			@callback.call(building)
		end
		
		def onAdded(host)
			onStatusChange(host.building?)
		end
	end
	
	class NotifyRebuild < Notify::Callback
	end
	
	attr_reader :path
	
	def initialize
		@albums = {}
		@albums.extend(Mutex_m)
    @albums_incremental = []
		@tracks = []
		@tracks.extend(Mutex_m)
    @built_mutex = Mutex.new
		@built_condition = ConditionVariable.new
		@build_status_mutex = Mutex.new
		@built = false
		@building = false
	end
	
  def self.valid_media_extensions
    ['mp3', 'flac']
  end
  
	def addTrack(track)
		album = albumForTrack(track)
		album.addTrack(track)
		
		@tracks.synchronize do
			@tracks << track
		end
		
		track
	end
	
	def addTrackFromPath(path)
		begin
			addTrack(Track.get(path))
		rescue Resource::Exception=>e
			puts "Music collection: #{e}"
		end
	end
	
  def albums_length
    wait_ensure_build()
    
    @albums.synchronize do
      @albums.length
    end
  end
  
	def albumForTrack(track)
		@albums.synchronize do
			album = @albums[track.albumTitle]
		
			if !album
				album = Album.new(track.albumTitle)
				@albums[track.albumTitle] = album
        @albums_incremental << album
			end
			
			album
		end
	end

	def building?
    @build_status_mutex.synchronize do
		  @building
	  end
	end

	def files
		raise "Please return an array from this method."
	end

	def rebuild
	  log { "Rebuild request for music collection '#{self}'." }
    @buildThread.exit if @buildThread
    
    @build_status_mutex.synchronize do 
      @built = false
      @building = false
    end
    
		notify_call(NotifyRebuild, self)
	end

	def all_tracks!
    wait_ensure_build()

		@tracks
  end

	def each_track
		build if needs_build?
		
		@tracks.synchronize do
			return @tracks if @tracks.empty? && @built
		end
		
		finish = false
		lastIdx = 0
		until finish
		  yielded_tracks = nil
		    
			@tracks.synchronize do
        yielded_tracks = @tracks[lastIdx...@tracks.length]
		  end
			
		  if yielded_tracks
        yielded_tracks.each do |track|
          yield(track)
        end
		  end
		  
			finish = true if SafeThread.exit?
			
      @tracks.synchronize do
			  finish = true if !@building && lastIdx == @tracks.length
      end
			
			lastIdx = @tracks.length

      sleep(1) # tbd: wait for async event instead of sleep
		end
	end

  def more_albums(idx, max)
    if needs_build?
      build
      return [nil, []]
    end
  
    result_albums = nil
    result_idx = nil
    
    return [nil, []] if @albums_incremental.empty? && @built
    return [nil, []] if !@building && idx == @albums_incremental.length

    num = [@albums_incremental.length - idx, max].min
    result_albums = @albums_incremental[idx...idx + num]
    result_idx = idx + result_albums.length
    
    [result_idx, result_albums]
  end

  def track_at_idx(idx)
    if needs_build?
      build
    end

    loop do
      could_return_track = @built || @tracks.synchronize do
        @tracks.length > idx
      end

      break if could_return_track

      sleep(0.5)
    end

    @tracks.synchronize do
      if @tracks.length > idx
        @tracks[idx]
      else
        nil
      end
    end
  end

  def more_tracks(idx, max)
    if needs_build?
      build
      return [nil, []]
    end

    result_tracks = nil
    result_idx = nil
    
    @tracks.synchronize do
      return [nil, []] if @tracks.empty? && @built
      return [nil, []] if !@building && idx == @tracks.length

      num = [@tracks.length - idx, max].min
      result_tracks = @tracks[idx...idx + num]
      result_idx = idx + result_tracks.length
    end
    
    [result_idx, result_tracks]
  end
  
  def more_tracks_with_albums(idx, max)
    result = more_tracks(idx, max)
    
    tracks_replacement = result.last.collect do |track|
      [track, albumForTrack(track)]
    end
    
    [result.first, tracks_replacement]
  end
  
  def tracks_length
    wait_ensure_build()
       
    @tracks.synchronize do
      @tracks.length
    end
  end
  
	def inspect
		"MusicCollection of type #{self.class}"
	end

  def viewable_clean_verify?
    false
  end
  
  def viewable_dirtyTestReference
    @tracks.length()
  end
  
protected
  def wait_ensure_build
    if needs_build?
      build()
      @built_mutex.synchronize do
        @built_condition.wait(@built_mutex)
      end
    end
  end

  def needs_build?
    @build_status_mutex.synchronize do
      return true if @building
      return true if !@built
    end
    
    false
  end

	def build
    @built_mutex.synchronize do
      @build_status_mutex.synchronize do
  		  return if @building
  		
        @building = true
  		  @built = false
      end
      
      @albums.synchronize do
        @albums.clear
        @albums_incremental.clear
      end
      
      @tracks.synchronize do
        @tracks.clear
      end
      
  		log { "Building music collection..." }
  		
  		@buildThread = SafeThread.new {
  			notify_each(NotifyBuildStatus) { |n| n.onStatusChange(true) }
  			
  			self.files.each do |f|
  				addTrackFromPath(f)
  				break if SafeThread.exit?
  			end
  
        @build_status_mutex.synchronize do
          @building = false
  		  end
  			
        if !SafeThread.exit?
  			  log { "Music collection is built." }
  			  
          @build_status_mutex.synchronize do 
  			    @built = true
          end
  			else
          puts "Music collection build aborted."
        end
  			
  			notify_each(NotifyBuildStatus) { |n| n.onStatusChange(false) }
      
  		  @built_condition.broadcast
  		}
		end
	end
end

class MusicCollectionM3U < MusicCollection
	attr_reader :files

	def initialize(path)
		super()
		
		@files = []
		
		@path = path
		puts "Loading M3U playlist at #{@path}..."
		@path.to_file.each do |line|
		  path = Path.new(line.chomp)
		  if self.class.valid_media_extensions.include?(path.extension)
        @files << path
		  end
		end
	end
	
	def viewable_description
		@path.to_s
	end
end

class MusicCollectionPath < MusicCollection
	class Files
		include ConfigYAML

    attr_accessor :rebuild

		persistent :array

		def initialize(paths)
			super()

			@paths = Array(paths).collect do |path|
        path.sub_invalid.to_sys.gsub(/\/|\\/,'_')
      end
      
			@array = []
      @rebuild = false
		end

		def array
			@array
		end

		def configElementName
			"MusicCollection_#{@paths}"
		end
	end
	
	def initialize(path)
		super()
		@path = path
		@files = Files.new(path)
	end
	
	def estimatedSize
		sleep(1) while !@scannedFiles
		return @tracks.length if !@tracks.empty?
		return self.files.length
	end

  def rebuild
    @files.rebuild = true
    super
  end

	def files
    success = false
    
		if !@files.rebuild && @files.array.empty?
			success = begin
        puts 'Loading file list...'
				@files = @files.load
				puts 'File list loaded.'
        
        true
			rescue DLB::Config::ExceptionLoad=>e
        puts "Load failed: #{e}"
        false
		  end
    end

    if !success
      puts "Retrieving file list at #{@path}..."
      @files = Files.new(@path)
      
      paths = Array(@path)
      
      paths.each do |path|
        files = (path + Path.to_file("**/*.*")).ls.select do |path|
          self.class.valid_media_extensions().include?(path.extension)
        end
        
        @files.array.concat(files) 
      end

      paths.select! { |path|  } 

      puts "Caching file list to disk..."
      
      @files.array.sort!
      @files.save
      
      puts "File list cached to #{@files.class.configPath}."
		end
		
    @files.rebuild = false
    
		@files.array
	end

	def viewable_description
		@path.to_s
	end
end
