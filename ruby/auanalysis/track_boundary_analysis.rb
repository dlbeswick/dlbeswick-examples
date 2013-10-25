require 'thresholding'
require 'wavelet'
require 'std/view/Viewable'
require 'narray_auanalysis'

# This can be enabled for systems with very low amounts of memory. tbd: make app option.
$wavelet_use_disk = false

def selection_sort(a, left, size, step)
  i = left
  while i < left + (size-1) * step do
    min = i
    j = i + step
    while j < left + size * step do
      min = j if a[j] < a[min]
      j += step
    end
    tmp = a[i]
    a[i] = a[min]
    a[min] = tmp
    i += step
  end
end

def triplet_adjust(a, i, step)
  j = i + step
  k = i + 2 * step
  
  if a[i] < a[j]
    if a[k] < a[i]
      tmp = a[i]
      a[i] = a[j]
      a[j] = tmp
    elsif a[k] < a[j]
      tmp = a[j]
      a[j] = a[k]
      a[k] = tmp
    end
  else
    if a[i] < a[k]
      tmp = a[i]
      a[i] = a[j]
      a[j] = tmp
    elsif a[k] > a[j] 
      tmp = a[j]
      a[j] = a[k]
      a[k] = tmp
    end
  end
end

def approximate_median(a)
  size = a.length
  left_to_right = false
  left = 0
  step = 1
  threshold = 5
  while size > threshold do
    left_to_right = !left_to_right
    rem = size % 3
    
    i = if left_to_right
      left 
    else
      left + (3 + rem) * step
    end
    
    (size / 3 - 1).times do
      triplet_adjust(a, i, step)
      i = i + 3 * step
    end
    
    if left_to_right
      left = left + step
    else
      i = left
      left = left + (1 + rem) * step
    end
    
    selection_sort(a, i, 3 + rem, step)
    
    if rem == 2
      if left_to_right
        tmp = a[i + step]
        a[i + step] = a[i + 2 * step]
        a[i + 2 * step] = tmp
      else
        tmp = a[i + 2 * step]
        a[i + 2 * step] = a[i + 3 * step]
        a[i + 3 * step] = tmp
      end
    end
    
    step = 3 * step
    size = size / 3
  end
  
  selection_sort(a, left, size, step)
  
  a[left + step * (size - 1) / 2]
end

class WaveletReconstruction 
  include DataSource
  include WaveletWithSoundFile
  
  def initialize(wavelet)
    @wavelet = wavelet
  end

  def calc(data)
    wavelet_result = @wavelet.calc(data)
    
    reconstructed = NArray.sfloat(wavelet_result.length)
    
    work = NArray.sfloat(wavelet_result.length)
      
    @wavelet.native.invert(wavelet_result.to_buffer, wavelet_result.length, reconstructed.to_buffer, wavelet_result.length, work.to_buffer, work.length)
    reconstructed
  end
  
  def map_range(sample_range, num_samples)
    data = @wavelet.source.map_range(sample_range, sample_range.length)
    calc(mix_data!(data))
  end

  def domain
    @wavelet.source.domain
  end

  def range
    @wavelet.source.range
  end
end

class TrackBoundaryAnalysis
  include ModuleLogging
  include SymbolDefaults
  include Viewable
  
  default_accessor :boundaries, []
  default_accessor :hold_seconds, 0.07
  default_accessor :release_seconds, 1.0
  default_accessor :leadout_seconds, 3.0
  default_accessor :threshold_test_proportion, 0.0
  default_accessor :sample_ms, 50
  default_accessor :step_ms, 25
  default_accessor :hit_threshold_db, -60
  default_accessor :miss_threshold_mod_db, 0.0
  attr_infer :denoiser, Bandpass.new
  attr_infer :wavelet do WaveletDiscreteHaar.new; end

  # tbd: wavelets don't have editable attributes, but should be able to choose type of wavelet
  # allattributesclassselector won't work because no editable attributes are available
  no_view :wavelet 
  
  symboldefaults_init
  
  def calc(sound_file, range = (0...sound_file.length_samples))
    @boundaries = calc_boundaries(sound_file, range)
  end
  
  def calc_boundaries(sound_file, range)
    analyze(sound_file, range)
  end
  
  def clear_cached_transforms
    if $wavelet_use_disk
      @wavelet_transformed_soundfile.close if @wavelet_transformed_soundfile && !@wavelet_transformed_soundfile.closed?
    end
    @wavelet_transformed_soundfile = nil
  end
  
  def domain
    (0...@sound_file.length_samples)
  end
  
  def range
    (0..1)
  end
  
  def threshold_factors_for_data(sound_file, range_samples)
    step_samples = sound_file.seconds_to_samples(step_ms() / 1000.0)
    threshold_amplitude = Mapping::Gain_db_Inverse.calc(hit_threshold_db(), 0, 1)
    
    transformed_data = transform_for_threshold_analysis(sound_file)
    transformed_data = WaveletResultMemoryMapped.sfloat(transformed_data.path)

    result = []
    
    range_samples.first.step(range_samples.last, step_samples) do |i|
      data = transformed_data.refer_slice(i...i+[step_samples, range_samples.last - i].min)
      factor = calc_threshold_factor(data, threshold_amplitude)
      result << [i, factor]
    end
    
    transformed_data.close
    
    result
  end
  
  def transform_for_threshold_analysis(sound_file)
    transformed = wavelet_transformed_soundfile(sound_file)
    
    xx 'begin threshold'
    $stdout.flush
    Progress.run("Thresholding") do
      denoiser().run(transformed)
    end
    xx 'end threshold'    
    
    transformed.close if transformed.respond_to?(:close)

    result = nil

    Progress.run("Reconstruction") do
      if $wavelet_use_disk
        reconstructed_path = DLB::Tempfile.ready('boundary_reconstructed')
        wavelet.invert_disk(transformed.path, reconstructed_path.path)
        result = reconstructed_path
      else
        result = wavelet.invert(transformed)
      end
    end
    
    result
  end

  def wavelet_transformed_soundfile(sound_file)
    @wavelet_transformed_soundfile ||= begin
      sound_file_disk_io = DLB::Tempfile.new('boundary_input')
      
      Progress.run(sound_file.length_samples, "Extracting sound data") do |progress|
        sound_file.sequential_reader.write_mono_to(sound_file_disk_io)
      end
      
      sound_file_disk_io.close
      
      ObjectSpace.garbage_collect
      
      wavelet = wavelet()

      result = nil

      Progress.run("Wavelet transform") do
        if $wavelet_use_disk
          transformed_path = DLB::Tempfile.ready('boundary_transformed')
          wavelet.calc_disk(sound_file_disk_io.path, transformed_path.path)
          result = transformed_path
        else
          result = wavelet.calc(NArray.to_na(File.read(sound_file_disk_io.path.to_sys), NArray::SFLOAT))
        end
      end
      
      ObjectSpace.garbage_collect
      
      if $wavelet_use_disk
        WaveletResultMemoryMapped.sfloat(transformed_path.path)
      else
        result
      end
    end

    if $wavelet_use_disk
      @wavelet_transformed_soundfile.reopen if @wavelet_transformed_soundfile.closed?
    end

    @wavelet_transformed_soundfile
  end
  
protected
  def calc_threshold_factor(data, threshold_amplitude)
    data.abs.ge(threshold_amplitude).where.length / data.length.to_f
  end

  def first_sample_hit_threshold(data, offset, threshold_amplitude)
    where = data.abs.ge(threshold_amplitude).where
    
    if where.empty?
      nil
    else
      offset + where.first
    end
  end
  
  def first_sample_miss_threshold(data, offset, threshold_amplitude)
    where = data.abs.lt(threshold_amplitude).where
    
    if where.empty?
      nil
    else
      offset + where.first
    end
  end
  
  def track_start_time_for(sound_file, threshold_hit_time)
    [0, threshold_hit_time - sound_file.seconds_to_samples(0.01)].max
  end
  
  def analyze(sound_file, range_samples = nil)
    result = []
      
    hold_samples = sound_file.seconds_to_samples(@hold_seconds).ceil
    release_samples = sound_file.seconds_to_samples(@release_seconds).ceil
    leadout_samples = sound_file.seconds_to_samples(@leadout_seconds).ceil
    range_samples = (0...sound_file.length_samples) if !range_samples
    
    leadout_hit_time = nil
    threshold_hit_duration = 0
    threshold_hit_time = nil
    threshold_miss_time = nil
    threshold_miss_duration = 0
    threshold_miss_factor = nil
    track_found = false
    
    step_size = sound_file.seconds_to_samples(@step_ms / 1000.0).ceil
    sample_size = sound_file.seconds_to_samples(@sample_ms / 1000.0).ceil
    hit_threshold_value = Mapping::Gain_db_Inverse.calc(@hit_threshold_db, 0, 1)

    if $wavelet_use_disk
      reconstructed_tmpfile = transform_for_threshold_analysis(sound_file)
      reconstructed_tmpfile.open
    else
      reconstructed_data = transform_for_threshold_analysis(sound_file)
    end
    
    range_samples = (range_samples.first..[range_samples.normalise_inclusive.last,sound_file.length_samples].min)
    
    ProgressStdout.new(range_samples.last - range_samples.first, "Track boundary analysis.", 5) do |progress|
      range_samples.first.step(range_samples.last, step_size) do |i|
        reconstructed_tmpfile.seek(i * 4) if $wavelet_use_disk
        read_samples = [sample_size, range_samples.last - i].min

        data = 
          if $wavelet_use_disk
            NArray.to_na(reconstructed_tmpfile.read(read_samples * 4), NArray::SFLOAT, read_samples)
          else
            reconstructed_data[i...i + read_samples]
          end
        
        threshold_factor = calc_threshold_factor(data, hit_threshold_value)
        
        did_hit_threshold = threshold_factor > @threshold_test_proportion
        first_hit_sample = if did_hit_threshold
          first_sample_hit_threshold(data, i, hit_threshold_value)
        else
          nil
        end

        if first_hit_sample != nil
          threshold_hit_time = first_hit_sample if threshold_hit_time == nil 
          threshold_miss_time = nil
          threshold_hit_duration += step_size
          threshold_miss_duration = 0
        else
          threshold_hit_time = nil
          
          first_miss = first_sample_miss_threshold(data, i, hit_threshold_value)
          
          if threshold_miss_time == nil
            if first_miss != nil
              threshold_miss_time = first_miss
            else
              threshold_miss_time = i
            end
          end
          
          threshold_hit_duration = 0
          threshold_miss_duration += step_size
        end

        if !track_found
          if threshold_hit_duration >= hold_samples
            track_found = true
            boundary_time = track_start_time_for(sound_file, threshold_hit_time) 
            log { "#{threshold_factor} at #{sound_file.samples_to_seconds(threshold_hit_time)} (track at #{sound_file.samples_to_seconds(boundary_time)})" }
            result << [boundary_time, threshold_factor]
            leadout_hit_time = nil
          elsif !result.empty? && leadout_hit_time == nil
            if threshold_miss_duration >= leadout_samples
              log { "Leadout hit at #{sound_file.samples_to_seconds(i)}" }
              leadout_hit_time = i
            end
          end
        else # track found
          if threshold_miss_duration >= release_samples
            track_found = false
            threshold_miss_factor = threshold_factor
            log { "Threshold miss #{threshold_miss_factor} at #{sound_file.samples_to_seconds(i)}" }
          end
        end
        
        progress << step_size
      end
    end
  
    if leadout_hit_time == nil
      log { "End of file reached without leadout hit, recording file end as leadout." }
      result << [range_samples.last_value, 1.0] 
    else    
      log { "End marker at #{sound_file.samples_to_seconds(leadout_hit_time)}" }
      result << [leadout_hit_time, threshold_miss_factor] 
    end

    clear_cached_transforms()
        
    result
  end
end

require 'std/view/user_actions'

class ViewTrackBoundaryAnalysis < Views::UserActionsPopup
  suits_class(TrackBoundaryAnalysis)
end
