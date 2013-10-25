require 'std/view/user_actions'
require 'std/view/Viewable'
require 'std/raise'

class BatchExporter
  include Viewable
  
  attr_reader :available
  attr_reader :library
  attr_reader :export_queue
  
  def initialize(library)
    @export_queue = []
    @library = library
  end

  def possibilities
    @library.source_albums
  end
    
  def queue(source_album)
    raise "Already queued." if @export_queue.include?(source_album)
    @export_queue << source_album
  end

  def unqueue(source_album)
    raise "Not present." if !@export_queue.include?(source_album)
    @export_queue.delete(source_album)
  end
  
  def viewable_dirtyTestReference
    @export_queue.collect{ |source_album| source_album.to_s }.join
  end
end

class ViewBatchExporter < Views::UserActionsButtons
  unenumerable
  
  suits(BatchExporter)
  
  custom_view do
    @possibilities = DataListbox.new(self, target().possibilities.reverse) do |data|
      if data
        target().queue(data)
        target().modified(self)
        generate
      end
    end
    layoutFillHorizontal(@possibilities.gui)
    
    @queued = DataListbox.new(self, target().export_queue) do |data|
      if data
        target().unqueue(data)
        target().modified(self)
        generate
      end
    end
    layoutFillHorizontal(@queued.gui)
    
    self
  end
  
  def export
    export_tracks_desired = []
      
    target().export_queue.each do |source_album|
      export_tracks_desired += source_album.source_tracks
    end
    
    ViewContainers::Single.new(ExportRequest.new(export_tracks_desired), self, ViewSourceExportRequestExporter)
  end
  
  def remove_existing_outputs
    desired_export_tracks = target().export_queue.collect{ |source_album| source_album.source_tracks }.flatten
      
    begin
      $app.exporter.existing_outputs(desired_export_tracks).each do |path|
        path.delete
      end
    rescue ::Exception=>e
      err { "#{e}\n#{e.backtrace}" }
    end
  end
  
  def generateExecute
    @possibilities.data = target().possibilities.reverse - target().export_queue
    @queued.data = target().export_queue
  end
end
