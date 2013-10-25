require 'std/view/user_actions.rb'
require 'transform/export_request.rb'

class ViewSourceExportRequestExporter < Views::UserActionsButtons
  include ModuleLogging
  
  suits_class(ExportRequest)
  unenumerable # this is not a general-purpose viewer, it supports the specific UI function of exporting a set of source tracks

  custom_view do
    layoutAddVertical(View.new_for(target().export_entries, self))
      
    layoutAddVertical makeStaticText(self, "Destination files:")
    
    # use an attribute so that the array view can update automatically when the export list changes 
    @files = AttributeReference::Object.new([])
      
    calculate_outputs()
    
    layoutAddVertical(View.new_for(@files, self))

    layoutAddVertical makeStaticText(self, "Max parallel tasks:")
    layoutAddVertical(View.viewAttribute(:max_parallel_tasks, target(), self))
  end
  
  def calculate_outputs
    formats_to_outputs = $app.exporter.output_formats_to_outputs(target().exportable_source_tracks)
      
    output_paths = formats_to_outputs.values.collect { |outputs| outputs.collect { |output| output.dest_path } }.flatten
    
    @files.set(output_paths)
    @files.modified(self)
  end
  
  def export
    begin
      $app.exporter.export_tracks_batch_parallel(target().max_parallel_tasks, target().exportable_source_tracks)
      
      true
    rescue ::Exception=>e
      err { "#{e}\n#{e.backtrace.join($/)}" }
        
      false
    end
  end
  
  def export_serial
    begin
      target().exportable_source_tracks_by_source_album.each do |source_album, album_entries|
        $app.exporter.export_album(source_album, album_entries)
      end
      
      true
    rescue ::Exception=>e
      err { "#{e}\n#{e.backtrace.join($/)}" }
        
      false
    end
  end
  
  def export_and_quit
    export_result = export()
    exit if export_result == true
  end
  
  def show_dest
    outputs = $app.exporter.output_formats_to_outputs(target().source_tracks).values.flatten
    directories = outputs.collect{ |output| output.dest_path.no_file }.uniq
        
    directories.each do |path|
      PlatformOS.shell_open(path) 
    end
  end

  def remove_existing_outputs
    begin
      $app.exporter.existing_outputs(target().exportable_source_tracks).each do |path|
        path.delete
      end
    rescue ::Exception=>e
      err { "#{e}\n#{e.backtrace}" }
    end
  end
  
  def rewrite_metadata
    target().source_tracks.each do |source_track|
      $app.exporter.rewrite_metadata(source_track)
    end
  end
end
