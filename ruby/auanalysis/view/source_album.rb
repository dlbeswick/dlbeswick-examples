require 'media/source_album'
require 'transform/export_request'
require 'transform/task_runner'
require 'transform/export/task_album_boundary_analysis'
require 'std/view/array_combo'
require 'std/view/messagebox'
require 'std/view/user_actions'

class ViewSourceAlbum < Views::UserActionsPopup
  include ModuleLogging
  suits_class(SourceAlbum)

  def clean_artist_names
    if target().artist
      target().artist = 
        target().library.artists.add_or_retrieve(
          Library.cleaned_artist_name(target().artist.name)
        )
    end
        
    target().tracks.each do |track|
      if track.artist
        track.artist = target().library.artists.add_or_retrieve(
          Library.cleaned_artist_name(target().artist.name)
        )
      end
    end
    
    target().modified(self)
  end
  
  def clean_track_titles
    target().tracks().each do |track|
      track.title = Library.cleaned_title(track.title)
    end
    
    target().modified(self)
  end 
  
  def source_files_add_transient
    selected_path = GUI.user_get_dir(self)
    if selected_path
      possibilities = selected_path.ls.select { |path| SourceFile.new_for_path?(path) }
      
      if !possibilities.empty?
        target().source_files.clear
        
        possibilities.each do |path|
          target().source_files << SourceFile.new_for_path(target(), path, true)
        end
        
        target().modified(self)
        true
      else
        ViewContainers::Single.new(GUIMessage.new("No potential source files found in that folder."), self)
        false
      end
    end
  end

  def source_tracks_sort_by_filename
    target().tracks.sort_by! do |track| 
      track.source_track(target()).source_file.path
    end

    target().tracks.each_with_index do |track, idx|
      track.order = idx + 1
    end

    target().modified(self)
  end

  def source_files_import_transient
    target().tracks = []
    target().source_tracks = []
      
    if source_files_add_transient()
      target().source_tracks = target().extract_source_tracks

      if !target().source_tracks.empty?
        target().title = target().source_tracks.first.album_description
        target().release_date = target().source_tracks.first.album_release_date
        target().media_quantity = target().source_tracks.length
      end

      if target().tracks.empty?
        recreate_tracks()
      end
      
      target().sort_tracks
      
      begin    
        target().infer_album_artist_from_track_data
      rescue SourceAlbum::Exception
      end
      
      target().modified(self)
    end
  end

  def source_files_clear
    target().source_files.clear
    target().modified(self)
  end
  
  def detect_album_tracks
    if target().source_tracks.empty?
      err { "No source tracks defined -- extract source files first." }
      return
    end
    
    target().tracks = target().extract_album_tracks()

    begin    
      target().artist = target().infer_album_artist_from_track_data
    rescue SourceAlbum::Exception
    end
    
    target().tracks().modified(self)
    modified(self)
  end
  
  def extract_and_set_track_boundaries
    ViewContainers::Single.new(attribute(), GUI.main_window, ViewSourceAlbumTrackBoundaries)
  end
  
  def extract_and_set_track_boundaries_then_export
    analyzers = target().source_files.collect do
      TrackBoundaryAnalysis.new
    end
    
    analyzers.zip(target().source_files) do |analyzer, source_file|
      analyzer.calc(source_file.sound_file)
    end
    
    if ViewSourceAlbumTrackBoundaries.boundary_count_matches_tracks(target(), analyzers) == false
      ViewContainers::Single.new(GUIMessage.new("Warning: #{ViewSourceAlbumTrackBoundaries.boundaries_count(analyzers)} boundaries found, but #{target().tracks.length} tracks are defined."), self)
    end
    
    ViewSourceAlbumTrackBoundaries.set_source_tracks_from_boundaries(target(), analyzers)

    $app.exporter.export_album(target(), target().source_tracks)
  end
  
  def discogs_metadata
    ViewContainers::Single.new(attribute(), self, ViewDiscogsResults)
  end
  
  def export_tracks
    ViewContainers::Single.new(ExportRequest.new(target().source_tracks), self, ViewSourceExportRequestExporter)
  end
  
  def fix_track_references
    target().source_tracks.zip(target().tracks).each do |source_track, track|
      source_track.track = track
    end
    target().modified(self)
  end
    
  def recreate_tracks
    target().clear_tracks
    target().source_tracks.each_with_index do |source_track, i|
      track = Track.new(target())
      track.title = source_track.description()
      track.title ||= "#{source_track.source_file.path.file.no_extension}#{index}"
    end
    
    fix_track_references()
    target().modified(self)
  end
  
  def reimport
    target = target()
    
    target.library.reimport(target)
    
    attribute().modified(self)
    target.library.modified(self)
  end

  def reset_track_ordering
    # re-order tracks by the order of their reference in the source tracks list
    target().tracks = target().tracks.sort_by do |track|
      track.order = nil
      source_track = target().source_tracks.find { |x| x.track == track }
      target().source_tracks.index(source_track)
    end
    target().modified(self)
  end
  
  def source_tracks_from_source_files
    target().source_tracks = target().extract_source_tracks
    
    if target().source_tracks.empty?
      # extraction failed, construct a basic source track using the whole source file
      target().source_files().each do |sf|
        source_track = SourceTrack.new(target(), sf, sf.path.file.literal)
        source_track.sample_range = (0...sf.sound_file.length_samples)
        target().source_tracks << source_track
      end
      
      target().source_tracks.sort! { |x, y| x.description <=> y.description }
    end
    
    fix_track_references()
    target().modified(self)
  end

  def self.order
    1
  end
end

class ViewDiscogsResults < ViewBasicAttributeReference
  unenumerable
  
  suits(SourceAlbum)
  
  def self.order
    -100
  end
  
protected
  def do_merge
    if @combo.selected
      target().set_from(@combo.selected)
      target().modified(self)
    end
  end
  
  def make_controls
    super
    
    layoutAddVertical makeStaticText(self, "Please wait, querying Discogs...")
    
    guiAdded()
    
    begin
      discogs = Auanalysis::Discogs.new
    
      albums = if !target().catalogue_id().empty?
        discogs.with_catalogue_id(target().catalogue_id())
      else
        discogs.with_artist_and_title(target().artist, target().title())
      end
    rescue Auanalysis::Discogs::Exception => e
      layoutAddVertical makeStaticText(self, "HTTP request failed. Please try again. (#{e.message}\n\n#{e.backtrace.join("\n")})")
      return
    end
      
    @combo = View.new_for(albums, self, ViewArrayCombo)
    layoutAddVertical(@combo)
    
    button = makeButton(self, "Merge") do
      do_merge()
    end
    layoutAddHorizontal(button)
    
    guiAdded()
  end
end

class ViewSourceAlbumTrackBoundaries < View
  unenumerable
  
  suits(SourceAlbum)
  
  include Views::AttributeReferenceView
  
  def add_source_file_analyzer_view_pairs
    @analyzers.zip(target().source_files) do |analyzer, source_file|
      sound_file = source_file.sound_file
  
      layoutAddVertical(View.new_for(sound_file, self))
      
      layoutAddVertical(View.new_for(analyzer, self))
    end
  end
  
  def make_controls
    super

    @analyzers = []

    target().source_files.each do |source_file|
      @analyzers << TrackBoundaryAnalysis.new
    end

    # alias
    analyzers = @analyzers
    
    button = makeButton(self, "Clear xforms") do
      analyzers.each do |analyzer|
        analyzer.clear_cached_transforms
      end
    end
    layoutAddHorizontal(button)
    
    button = makeButton(self, "Wavelet xforms") do
      analyzers.zip(target().source_files) do |analyzer, source_file|
        transformed = analyzer.wavelet_transformed_soundfile(source_file.sound_file)
        
        puts
        puts source_file.path.to_s
          
        maxes = []
        means = []
        medians = []
        rmses = []
        percentiles = []
          
        percentile_divisions = 20
          
        (0...transformed.length_vectors / 2).each do |idx|
          vector = transformed.refer_vector(idx).abs
          vector.sort!
          maxes << format("%.2f", Mapping::Gain_db.calc(vector.max))
          means << format("%.2f", Mapping::Gain_db.calc(vector.mean))
          medians << format("%.2f", Mapping::Gain_db.calc(vector[vector.length / 2]))
          rmses << format("%.2f", Mapping::Gain_db.calc(vector.rms))
          percentiles << (0..percentile_divisions).collect do |i|
            format("%.2f", Mapping::Gain_db.calc(vector[Mapping.linear(i, 0.0, percentile_divisions, 0, vector.length-1)]))
          end
        end
        
        puts "Vec\t\tMax\t\tAvg\t\tMed\t\tRMS"
        (0...maxes.length).each do |idx|
          puts "#{idx}\t\t#{maxes[idx]}\t\t#{means[idx]}\t\t#{medians[idx]}\t\t#{rmses[idx]}"
        end
        
        puts
        
        (0..percentile_divisions).each do |p|
          $stdout.write "#{Mapping.linear(p, 0, percentile_divisions, 0, 100)}%\t\t"
        end
        puts
        
        percentiles.each do |each_percentile|
          each_percentile.each do |p|
            $stdout.write "#{p}\t\t"
          end
          puts
        end
        
        puts
      end
    end
    layoutAddHorizontal(button)
    
    button = makeButton(self, "Calculate for all.") do
      start_time = Time.now

      analyzers.zip(target().source_files) do |analyzer, source_file|
        log { "Analyzing #{source_file.path}" }
        analyzer.calc(source_file.sound_file)
      end

      analyzers.each do |analyzer|
        analyzer.modified(self)
      end

      log { "Analysis complete (#{format('%.2f', Time.now - start_time.to_f)} seconds)." }
    end
    layoutAddHorizontal(button)
    
    button = makeButton(self, "Calc for all (parallel)") do
      task = TaskAlbumBoundaryAnalysis.new(analyzers.zip(target().source_files))

      # Unfortunately, there's no speedup here because the operation is i/o bound.
      # 4 files in parallel on ssd = 70 secs, hd = 90 secs
      # 4 files serial on ssd = 70 secs, hd = 80 secs
      # hd is faster in the serial case, the same in the ssd case.
      TaskRunner.new.run(task)

      analyzers.zip(task.result).each do |analyzer, result|
        analyzer.boundaries = result
        analyzer.modified(self)
      end
    end
    layoutAddHorizontal(button)

    add_source_file_analyzer_view_pairs()
      
    button = makeButton(self, "Overwrite album's source tracks with result") do
      if ViewSourceAlbumTrackBoundaries.boundary_count_matches_tracks(target(), analyzers) == false
        ViewContainers::Single.new(GUIMessage.new("Warning: #{ViewSourceAlbumTrackBoundaries.boundaries_count(analyzers)} boundaries found, but #{target().tracks.length} tracks are defined."), self)
      end
      
      target().source_tracks  = []
        
      target_track_idx = 0

      analyzers.zip(target().source_files) do |analyzer, source_file|
        if analyzer.boundaries.length < 2
          ViewContainers::Single.new(GUIMessage.new("Only '#{analyzer.boundaries.length}' boundaries were found, but at least 2 are required."), self)
          break
        end
         
        # create and add source tracks and album tracks for each boundary
        source_tracks_for_source_file = []
          
        (analyzer.boundaries.length - 1).times do |idx|
          xx "make #{source_file.title}#{idx+1}"
          source_track = SourceTrack.new(target(), source_file, "#{source_file.title}#{idx+1}")
          
          target().source_tracks << source_track
          xx target().source_tracks.length
          source_tracks_for_source_file << source_track
          
          source_track.track = if target_track_idx < target().tracks.length
            target().tracks[target_track_idx]
          else
            Track.new(target())
          end
          
          target_track_idx += 1
        end
      
        # fill sample range for each source track with boundary data
        boundary_idx = 0
        
        source_tracks_for_source_file.each do |source_track|
          source_track.sample_range = (analyzer.boundaries[boundary_idx][0]...analyzer.boundaries[boundary_idx+1][0])
          boundary_idx += 1
        end
      end
      
      target().modified(self)
    end
    
    layoutAddVertical(button)
  end
  
  # tbd: move to common class
  def self.boundary_count_matches_tracks(target, analyzers)
    if target.tracks.empty?
      true
    else
      boundaries_count(analyzers) - analyzers.length == target.tracks.length
    end
  end
  
  # tbd: move to common class
  def self.boundaries_count(analyzers)
    analyzers.inject(0) { |sum, analyzer| sum += analyzer.boundaries.length }
  end
  
  # tbd: move to common class
  def self.set_source_tracks_from_boundaries(target, analyzers)
    target.source_tracks  = []
      
    target_track_idx = 0
  
    analyzers.zip(target.source_files) do |analyzer, source_file|
      if analyzer.boundaries.length < 2
        ViewContainers::Single.new(GUIMessage.new("Only '#{analyzer.boundaries.length}' boundaries were found, but at least 2 are required."), self)
        break
      end
       
      # create and add source tracks and album tracks for each boundary
      source_tracks_for_source_file = []
        
      (analyzer.boundaries.length - 1).times do |idx|
        source_track = SourceTrack.new(target, source_file, "#{source_file.title}#{idx+1}")
        
        target.source_tracks << source_track
        source_tracks_for_source_file << source_track
        
        source_track.track = if target_track_idx < target.tracks.length
          target.tracks[target_track_idx]
        else
          Track.new(target)
        end
        
        target_track_idx += 1
      end
    
      # fill sample range for each source track with boundary data
      boundary_idx = 0
      
      source_tracks_for_source_file.each do |source_track|
        source_track.sample_range = (analyzer.boundaries[boundary_idx][0]...analyzer.boundaries[boundary_idx+1][0])
        boundary_idx += 1
      end
      
      target.modified(self)
    end
  end
  
  protected
end
