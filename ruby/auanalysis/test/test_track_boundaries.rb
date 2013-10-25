require 'std/file_memory_mapped'
require 'sound_file_raw'
require 'thresholding'

class WaveletTest
  include ModuleLogging
  include SymbolDefaults
  include Viewable

  default_accessor :sample_length, 16384
  default_choices :source_file_path, [
    Path.new('test/track_boundary_test_0.aup'),
    Path.new('d:/data/recordings/natalie cole/good to be back/b.aup'),
    Path.new('d:/data/recordings/The Outside Agency/idpdz012/a.aup'),
    Path.new('d:/Data/Recordings/Ingler/pth005/a.aup'),
    Path.new('d:/Data/Recordings/Geroyche/Bitpop EP/a.aup'),
    Path.new('D:\Data\Recordings\Altern 8\Hypnotic St-8\a.aup'),
    Path.new('D:\Data\Recordings\Null Object\Hypnotic St-8\a.aup'),
    Path.new('D:\Data\Recordings\Dusko Lokin\Nitko Te Nece Voljeti Kao Ja\a.aup'),
    Path.new('D:\Data\Recordings\Christian Fischer\Clubtech EP\a.aup'),
    ]
  
  symboldefaults_init
  
  # hack for view, remove
  def recalc
    @wavelet = nil
    @sound_file = nil
    @reconstruction = nil
    wavelet()
  end
  
  def threshold(result, index, threshold)
    thresholded = result[index].to_a.collect do |e|
      if e.abs < threshold
        e = 0
      else
        e
      end
    end
    result[index] = Dsound::VectorFloat.new(thresholded)
  end
  
  def reconstruction
    @reconstruction ||= WaveletReconstruction.new(wavelet()).extend(WaveletWithSoundFile)
  end
  
  def sound_file
    @sound_file ||= SoundFile.from_path(@source_file_path)
  end
  
  def wavelet
    @wavelet ||= WaveletDiscreteHaar.new.extend(WaveletWithSoundFile)
  end
  
  def write_wavelet  
    step = sample_length()
    
    wavelet = wavelet()
  
    files_for_close = []

    wave_path = Path.new("wave")
    reconstruction_path = Path.new("./wavelet_raw_reconstruction")
    
    if sound_file().origin.newer_path?(wave_path) || wave_path.file_size / 2 != sound_file().length_bytes() / 2  
      output = wave_path.to_file("wb")
        
      Progress.run("Write wave") do
        SoundFileAudacity::SequentialReader.new(sound_file()).write_mono_to(output)
      end
      
      output.close
    else
      puts "#{wave_path} newer than #{sound_file().origin}, skipping wave write."
    end
    
    begin
      wave = NArrayMemoryMapped.sfloat(wave_path)
      files_for_close << wave

      # hack for file size, fix in mmap func
      File.new(Path.new("./wavelet_out").to_sys, 'w').close
      wavelet_out = NArrayMemoryMapped.sfloat(Path.new("./wavelet_out"), 'w+', sound_file().length_samples)
      files_for_close << wavelet_out
      wavelet_out.fill!(0)
      
      puts 'Start...'
      Progress.run("Wavelet") do
        wavelet.calc(wave, wavelet_out)
      end
      
      wave.close

      Progress.run("Process wavelet") do
        process_wavelet(wavelet_out)
      end
        
      # hack for file size, fix in mmap func
      File.new(reconstruction_path.to_sys, 'w').close
      reconstruction = NArrayMemoryMapped.sfloat(reconstruction_path, 'w+', sound_file().length_samples)
      files_for_close << reconstruction
      reconstruction.fill!(0)
      
      Progress.run("Reconstruction") do
        wavelet.invert(wavelet_out, reconstruction)
      end
      puts 'Done.'
      
      reconstruction_path
    ensure
      files_for_close.each { |io| io.close }
    end
  end
  
protected
  def process_wavelet(wavelet_output)
  end 
end

class ViewWaveletTest < Views::UserActionsButtons
  suits_class(WaveletTest)
  
  def onViewableDependencyEvent(record)
    target().recalc
    observe(attribute(), true)
  end
  
  def view_reconstruction
    container = ViewContainers::Single.new(target().reconstruction, self, ViewSoundFileAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sample_length()
    container.show
  end

  def view_sound_file
    container = ViewContainers::Single.new(target().sound_file, self, ViewSoundFileAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sound_file().length_samples()
    container.show
  end
  
  def view_wavelet
    container = ViewContainers::Single.new(target().wavelet, self, ViewSoundFileWaveletAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sample_length()
    container.show
  end
  
  def view_wavelet_difference
    container = ViewContainers::Single.new(target().wavelet, self, ViewSoundFileWaveletAmplitude, false)
    
    class << container.view.renderer
      def postprocess_sampled_data(data, desired_samples, drawing_area_width)
        if data.length >= 2
          data = data[0..1]
          
          (0...data[0].length/2).each do |i|
            data[1][i] = (data[1][i].abs - data[0][i*2].abs).abs
          end
          
          super([data[1]], desired_samples, drawing_area_width)
        else
          data
        end
      end
    end
    
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sample_length()
    container.show
  end
  
  def write_wavelet
    reconstruction_file = target().write_wavelet
    reconstruction_sound_file = SoundFileRaw.from_path(reconstruction_file, 1, 44800, 4)
    container = ViewContainers::Single.new(reconstruction_sound_file, self, ViewSoundFile, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = reconstruction_sound_file.length_samples
    container.show
  end
end

class ThresholdWaveletTest < WaveletTest
  include SymbolDefaults
  
  def wavelet
    @wavelet ||= ThresholdWaveletDiscreteHaar.new(sound_file()).extend(WaveletWithSoundFile)
  end
  
protected
  def process_wavelet(wavelet_output)
    return
    sorted_output = NArrayMemoryMappedTemp.new(wavelet_output.refer_slice(0...wavelet_output.length/2))
    sorted_output.sort!
    median = sorted_output[sorted_output.length / 2]
    sorted_output.close
    variance = median / 0.6745 # sparse way 11.85
    threshold = @wavelet.estimate_threshold_from_noise_variance(variance, wavelet_output.length)
    @wavelet.threshold_hard(wavelet_output, threshold)
  end 
end

class ViewThresholdWaveletTest < ViewWaveletTest
  suits_class(ThresholdWaveletTest)
end

class TrackBoundariesTest < WaveletTest
  attr_accessor :analysis
  
  default_view :analysis, "ViewAllAttributesReference"
  
  def initialize
    super
    @analysis = TrackBoundaryAnalysis.new
  end
  
  def execute
    sound_file = sound_file()
    
    labels = sound_file.labels.collect do |label|
      label.t = sound_file.seconds_to_samples(label.t)
      label
    end

    if labels.length < 2
      raise "Not enough labels in the source file."
    end
        
    @analysis.sound_file = sound_file 
    results = @analysis.calc

    count_mismatch = if results.length != labels.length
      err { "Bad number of tracks detected (result #{results.length} != #{labels.length})" }
      true
    else
      false
    end
    
    if !results.last == sound_file.length_samples()
      err { "Leadout calculated incorrectly (file end)." }
    end
    
    if !count_mismatch
      bad = []
      differences = []
      index = 0
        
      results[0..-2].zip(labels) do |result, label|
        index += 1
        
        result = result.first
        # allow more play before the label than after it
        if sound_file.samples_to_seconds(result - label.t) > 0.05 || sound_file.samples_to_seconds(label.t - result) > 0.25 
          bad << [index, label.t, result, "#{sound_file.samples_to_seconds(result - label.t) * 1000} ms" ]
        end
         
        differences << ["#{sound_file.samples_to_seconds(result - label.t) * 1000} ms"]
      end
      
      bad.collect! { |e| e.inspect }
      
      err { "Major discrepencies between specified and calculated boundaries: \n  #{bad.join("  \n")}\n" } if !bad.empty?
      
      log { "Differences: \n  #{differences.join("  \n")}" }

      bad.empty?
    else
      index = 0
      
      results.collect! do |result|
        index += 1
        "#{index}: #{result[1]} at #{result[0]} (#{sound_file.samples_to_seconds(result.first)} secs)"
      end
      
      log { "Results: \n  #{results.join("\n  ")}" }
        
      false
    end
  end
end

#SoundFile.log=3
class ViewTrackBoundariesTest < Views::UserActionsButtons
  suits_class(TrackBoundariesTest)

  def view_boundaries
    data = target().analysis().calc_boundaries(target().sound_file(), (0...target().sound_file().length_samples))
    data.insert(0, [0.0,0.0])
    data << [target().sound_file().length_samples(),0.0]
    container = ViewContainers::Single.new(data, self, GraphView, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sound_file().length_samples
    container.show
  end
  
  def view_sound_file
    container = ViewContainers::Single.new(target().sound_file.mappable_mono, self, ViewSoundFileAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sound_file().length_samples()
    container.show
  end
  
  def view_threshold_factor
    data = target().analysis.threshold_factors_for_data(target().sound_file(), (0...target().sound_file().length_samples))
    container = ViewContainers::Single.new(data, self, GraphView, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sound_file().length_samples
    container.show
  end
  
  def view_denoised
    data = NArrayMemoryMapped.sfloat(target().analysis.transform_for_threshold_analysis(target().sound_file).path)
    data.extend(SoundFileArray)
    container = ViewContainers::Single.new(data, self, ViewSoundFileAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = target().sound_file().length_samples
    container.show
  end
  
  def view_wavelet_db
    sound_file = target().sound_file
    
    sound_file_disk_io = DLB::Tempfile.new
    sound_file.sequential_reader.write_mono_to(sound_file_disk_io)
    sound_file_disk_io.close
    wavelet = WaveletDiscreteHaar.new
    result = WaveletResultMemoryMapped.sfloat(wavelet.calc_disk_tmpfile(sound_file_disk_io.path))
      
    window_width = target().analysis.sample_ms
    container = ViewContainers::Single.new(result, self, ViewSoundFileWaveletAmplitude, false)
    container.view.view_domain_start = 0
    container.view.view_domain_size = window_width
    container.show
  end
end
