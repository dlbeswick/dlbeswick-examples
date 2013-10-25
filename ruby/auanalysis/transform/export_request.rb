require 'std/view/Viewable'

class ExportRequest
  include SymbolDefaults
  include Viewable
  
  attr_reader :export_entries
  attr_infer :max_parallel_tasks, 9
  
  class ExportEntry
    include Viewable
    
    attr_accessor :export
    attr_accessor :source_track
    
    def initialize(source_track)
      @source_track = source_track
      @export = true
    end
  end
  
  def initialize(source_albums_or_tracks)
    @export_entries = []
      
    source_albums_or_tracks.each do |request|
      is_album = request.respond_to?(:source_tracks) 
      
      if is_album
        @export_entries += request.source_tracks.collect { |source_track| ExportEntry.new(source_track) }
      else
        @export_entries << ExportEntry.new(request)
      end
    end
  end
  
  def entries_by_album
    result = {}
      
    export_entries().each do |entry|
      array_for_key = result[entry.album] ||= []
      array_for_key << entry
    end
    
    result
  end

  def exportable_entries
    export_entries().find_all { |entry| entry.export == true }
  end
  
  def exportable_entries_by_source_album
    result = {}
      
    export_entries().each do |entry|
      if entry.export
        array_for_key = result[entry.source_track.source_album] ||= []
        array_for_key << entry if entry.export
      end
    end
    
    result
  end
  
  def exportable_source_tracks_by_source_album
    result = {}
      
    export_entries().each do |entry|
      if entry.export
        array_for_key = result[entry.source_track.source_album] ||= []
        array_for_key << entry.source_track
      end
    end
    
    result
  end

  def exportable_source_tracks
    exportable_entries().collect { |entry| entry.source_track }
  end
  
  def source_tracks
    export_entries().collect { |entry| entry.source_track }
  end
end
