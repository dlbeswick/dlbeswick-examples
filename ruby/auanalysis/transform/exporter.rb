require 'transform/output_format'
require 'transform/transform'
require 'transform/task_runner'
require 'transform/export/task_export_album'
require 'transform/export/task_export_batch_tracks'
require 'std/progress'

class Exporter
  include ConfigYAML
  include Raise
  include SymbolDefaults
  include Viewable
  
  persistent_accessor :default_dir_transform
  persistent_accessor :default_filename_transform
  persistent_accessor :output_formats
  
  default :default_dir_transform, <<-EOS
    if is_va?
      export_data.export_path + export_data.output_format.output_extension + Path.sub_invalid(true, title())
    else
      export_data.export_path + export_data.output_format.output_extension + Path.sub_invalid(true, artist().name) + Path.sub_invalid(true, title())
    end
  EOS
  
  default :default_filename_transform, '#{format(\'%.2d\', track().order)}. #{Path.sub_invalid(false, track().artist.name)} - #{Path.sub_invalid(false, track().title)}.#{export_data.output_format.output_extension}'
  
  default :output_formats, [
    OutputFormatFlac.new,
    OutputFormatMP3Duplicate.new,
    OutputFormatFlacDuplicate.new,
    OutputFormatMP3.new
  ]
  
  class Output
    attr_accessor :source_track
    attr_accessor :dest_path
  end
  
  symboldefaults_init
  
  def existing_outputs(source_tracks)
    output_formats().collect do |format|
      result = source_tracks.collect do |source_track|
        dest_path = source_track.dest_path(format)
        
        if dest_path.exists?
          dest_path
        else
          nil
        end
      end.compact
    end.flatten
  end
  
  # tbd: old code, remove
  def export_album_parallel(max_tasks, source_album, source_tracks=source_album.source_tracks)
    ProgressStdout.always_report = true # fix: UI code in data implementation
    
    existing_outputs = existing_outputs(source_tracks)  
    raise "Output files already exist: #{existing_outputs.join($/)}" if !existing_outputs.empty?
    
    runner = TaskRunner.new
    runner.run(TaskExportAlbum.new(self, source_album, source_tracks), max_tasks)
  end
  
  def export_tracks_batch_parallel(max_tasks, source_tracks)
    ProgressStdout.always_report = true # fix: UI code in data implementation
    
    existing_outputs = existing_outputs(source_tracks)  
    raise "Output files already exist: #{existing_outputs.join($/)}" if !existing_outputs.empty?
    
    runner = TaskRunner.new
    runner.run(TaskExportBatchTracks.new(self, source_tracks), max_tasks)
  end
  
  def export_album(source_album, source_tracks=source_album.source_tracks)
    ProgressStdout.always_report = true # fix: UI code in data implementation
    
    output_files = []
    
    if source_tracks.collect{ |t| t.source_album }.uniq.length != 1
      raise "All source tracks must be from the source album."
    end
      
    existing_outputs = existing_outputs(source_tracks)  
    raise "Output files already exist: #{existing_outputs.join($/)}" if !existing_outputs.empty?
      
    source_album.source_tracks.each do |source_track|
      output_files += export_track(source_track) if source_tracks.include?(source_track)
    end
    
    # be sure to use all of the album tracks here. that supports operations such as calculating album replaygain.
    output_formats_to_outputs(source_album.source_tracks).each do |output_format, outputs|
      output_paths = outputs.collect { |o| o.dest_path }
      output_format.post_export_album(output_paths, source_album)
    end
    
    source_album.source_tracks.each do |source_track|
      source_track.mark_processed
    end
  end
  
  def extract_raw_sound_data(source_track)
    source_track.source_file.extract_raw_sound_data(source_track)
  end
  
  def export_track(source_track)
    raw_track_tempfiles = extract_raw_sound_data(source_track)
    
    raw_track_paths = raw_track_tempfiles
      
    begin
      output_files = []  
        
      output_formats().each do |format|
        if format.valid_for?(source_track)
          output_path = source_track.dest_path(format)
          output_files << output_path
          format.export_track(output_path, raw_track_paths, source_track)
        end
      end
    ensure
      source_track.source_file.post_export(raw_track_paths)
    end
    
    output_files
  end

  def rewrite_metadata(source_track)
    output_formats().each do |format|
      if format.respond_to?(:write_metadata)
        output_path = source_track.dest_path(format)
        begin
          format.write_metadata(output_path, source_track)
        rescue Exception=>e
          err { e.to_s }
        end
      end
    end
  end
  
  def output_formats_to_outputs(source_tracks)
    output_format_to_files = {}

    output_formats().each do |output_format|
      source_tracks.each do |source_track|
        if output_format.valid_for?(source_track)
          ary = output_format_to_files[output_format] ||= []
            
          output = Output.new
          output.dest_path = source_track.dest_path(output_format)
          output.source_track = source_track
            
          ary << output
        end
      end
    end
    
    output_format_to_files
  end
    
  def output_format_for_extension(extension)
    result = output_formats().find { |format| format.output_extension == extension }
    raise "No output format object found for extension '#{extension}'." if !result
    result
  end
end
