require 'sound_file'
require 'sound_file_audacity_au'
require 'std/module_logging'
require 'rexml/document'
require 'narray_auanalysis'
require 'std/raise'

$has_narray = true

class SoundFileAudacity < SoundFile
  include Raise
  
  class FromXML
    include EnumerableSubclasses
    include ModuleLogging
    
    attr :parent
    
    def initialize(node, parent=nil)
      @parent = parent
      
      attributes = self.class.own_instance_methods
      
      node.attributes.each do |k, v|
        v = if /[^-0-9.+$]/.match(v)
          v
        else
          v.to_f
        end
        
        instance_variable_set("@#{k}", v)
      end

      node.elements.each do |element|
        name = element.name
        subclass = subclass_of_name(name)
        
        if !subclass
          warn { "No suitable class found for XML node '#{name}'" } 
        else
          new_obj = subclass.new(element, self)
          
          array = instance_variable_get("@#{name}")
          
          if !array
            array = []
            instance_variable_set("@#{name}", array)
          end
          
          array << new_obj
        end
      end
    end
    
    def self.own_instance_methods
      @own_instance_methods ||= instance_methods - FromXML.instance_methods
    end
    
    def subclass_of_name(name)
      @@subclasses ||= {}
      result = @@subclasses[name]
      
      if !result
        result = FromXML.subclasses().find { |x| /#{name}$/i.match(x.name) }
        @@subclasses[name] = result
      end
      
      result
    end
    
    def top_parent(who=self)
      if !who.respond_to?(:parent) || who.parent == nil
        who 
      else
        top_parent(who.parent)
      end
    end
  end
  
  class AudacityProject < FromXML
    attr :rate
    attr :wavetrack
    attr :labeltrack
    
    def all_waveclips
      wavetrack.collect do |wt|
        wt.waveclip.first
      end
    end
  end
  
  class LabelTrack < FromXML
    attr :label
  end
  
  class Label < FromXML
    attr_accessor :t
    attr :t1
    attr :title
  end
  
  class WaveTrack < FromXML
    attr :name
    attr :channel
    attr :linked
    attr :offset # Audacity 1.2 only -- find a way to make this safer
    attr :rate
    attr :sequence
    attr :waveclip # 1.3 only
    
    def data(file_handle_manager, sound_file_audacity, samples_range, num_samples)
      sequence[0].data(file_handle_manager, sound_file_audacity, samples_range, num_samples)
    end
  end

  # Audacity 1.3 only
  class WaveClip < FromXML
    attr :offset
    attr :sequence
    
    def data(file_handle_manager, sound_file_audacity, samples_range, num_samples)
      sequence[0].data(file_handle_manager, sound_file_audacity, samples_range, num_samples)
    end
  end
  
  class Sequence < FromXML
    include Raise
    
    attr :numsamples # Audacity 1.2
    attr :maxsamples # Audacity 1.3
    attr :sampleformat
    attr :waveblock

    def num_samples
      if @numsamples
        @numsamples
      elsif @maxsamples
        @maxsamples
      else
        raise "Neither numsamples nor maxsamples were defined for an Audacity Sequence node."
      end
    end
    
    def data(file_handle_manager, sound_file_audacity, samples_range, desired_samples)
      return [] if samples_range.length == 0
        
      possible_range = (0...num_samples())
      
      raise "Out of range (#{samples_range} outside #{possible_range})" if !possible_range.include_range?(samples_range)
      desired_samples = [samples_range.length, desired_samples].min 

      waveblocks = waveblocks_in_range(samples_range)
      
      use_summary = can_use_summary?(samples_range, desired_samples)
       
      if $has_narray && !use_summary
        available_samples = [num_samples(), desired_samples].min
          
        result = NArray.sfloat(samples_range.length)

        output_idx = 0
        
        in_range = waveblocks_in_range(samples_range)
        
        raise "No waveblocks in range for '#{samples_range}'." if in_range.empty? 
        
        in_range.each do |block|
          waveblock_data = block.relevant_data_for_project_range(file_handle_manager, sound_file_audacity, samples_range, desired_samples, false)
          if waveblock_data.length > 0
            result[output_idx...output_idx + waveblock_data.length] = waveblock_data
            output_idx += waveblock_data.length
          end
        end
        
        result
      else
        result = []
          
        waveblocks.each do |block|
          data = block.relevant_data_for_project_range(file_handle_manager, samples_range, desired_samples, true)
          result.concat(data)
        end

        result
      end
    end
    
    def can_use_summary?(samples_range, desired_samples)
      desired_samples <= samples_range.length / 256
    end
    
    def rate(sound_file_audacity)
      if sound_file_audacity.v1_2?
        parent.rate
      else
        parent.parent.rate
      end
    end
    
    def waveblock_idx_for_sample(sample)
      result = nil
      
      waveblock.each_with_index do |block, i|
        if block.start <= sample && block.end > sample
          result = i
          break
        end
      end
      
      raise "No waveblock for sample \##{sample}." if !result
      
      result
    end
    
    def waveblocks_in_range(samples_range)
      waveblock.find_all do |block|
        SoundFileAudacity.note { "Block #{block.project_range_samples} in range of #{samples_range}: #{block.in_range?(samples_range)}" } 
        block.in_range?(samples_range)
      end
    end
  end
  
  class WaveBlock < FromXML
    attr :start
    attr :simpleblockfile

    def relevant_data_for_project_range(file_handle_manager, sound_file_audacity, samples_range, num_samples, use_summary)
      intersection = project_range_samples() & samples_range

      return nil if intersection == nil
      
      sample_rate = num_samples / samples_range.length
      local_range = project_range_to_local(intersection)
      
      result = simpleblockfile.first.block_file(file_handle_manager, sound_file_audacity).data_samples(local_range, sample_rate * local_range.length)
        
      if use_summary && !result[0].respond_to?(:[])
        # if use of the summary was requested but summary (minmax) data was not returned, then it must have
        # been a block that was less than 256 samples on the tail end of a data request. 
        # Convert it to a minmax result.
        raise 'Bad assumption.' if result.length < 256 # tbd: remove this
        Resample.MinMax(result, 1)
      else
        result
      end
    end
    
    def end
      start() + len()
    end
    
    def in_range?(samples_range)
      (project_range_samples() & samples_range) != nil
    end

    def len
      simpleblockfile.first.len
    end
    
    def project_range_samples
      (start()...self.end)
    end
    
    def project_range_to_local(samples_range)
      Range.new(samples_range.first - start, samples_range.last - start, samples_range.exclude_end?)
    end
    
    def sampleformat
      parent.sampleformat
    end
  end
  
  class SimpleBlockFile < FromXML
    attr :filename
    attr :len
    attr :min
    attr :max
    
    def absolute_path(sound_file_audacity)
      if sound_file_audacity.v1_2?
        @absolute_path ||= top_parent.path_data.with_file(@filename)
      else
        subdir_a = Path.new(@filename[0..2] + "/")
        subdir_b = Path.new("d#{@filename[3..4]}/")
        @absolute_path ||= (top_parent.path_data + subdir_a + subdir_b).with_file(@filename)
      end
    end
    
    def block_file(file_handle_manager, sound_file_audacity)
      path = absolute_path(sound_file_audacity)

      # tbd: multithreading issue here?
      handle, is_new = file_handle_manager.handle_for(path, 'rb')
      
      if !@block_file
        @block_file = SoundFileAudacityAu.new(handle, path, sequence(), self, sound_file_audacity)
      end
      
      if is_new
        @block_file.reassign_io(handle)
      end
      
      @block_file
    end
    
    def sequence
      parent.parent
    end
  end
  
  class FileHandleManager
    def initialize
      @handles = {}
      @order = []
      @handle_limit = 20
    end
    
    def evict_oldest_handle
      handle = @order.first
      handle.close
      @handles.delete_if { |k, v| v == handle }
      @order = @order[1..-1]
    end
    
    def handle_for(path, mode)
      absolute_path = path.absolute
      handle = @handles[absolute_path]
      
      if !handle
        handle = path.to_file(mode)
        
        if @order.length >= @handle_limit
          evict_oldest_handle()
        end
        
        @handles[absolute_path] = handle
        @order << handle
        
        return handle, true
      else
        handle.seek(0)
        return handle, false
      end
    end
  end
  
  def self.from_io(io, path)
    self.new(io, path)
  end
  
  def self.can_open_path?(path)
    path.extension == "aup"
  end

  attr_accessor :cache_max_length_bytes
  attr :path

  def file_handle_manager
    @file_handle_manager ||= FileHandleManager.new
  end

  def initialize(io, origin)
    super(origin)

    @io = io
    
    raise "Audacity files must be opened by path." if !origin
    
  	@moduleLogging_id = "(#{origin.file})"
    
    contents = io.read
    
    # Audacity 1.2 files have "audacityproject" nodes, 1.3 has "project" nodes.    
    xml = REXML::Document.new(contents).elements['audacityproject']
      
    if xml
      @is_version_1_2 = true
    else
      @is_version_1_2 = false
      xml = REXML::Document.new(contents).elements['project']
    end

    raise "No 'audacityproject' (Audacity 1.2) or 'project' (Audacity 1.3) node found in file #{origin}" if !xml
    
    @project = AudacityProject.new(xml, self)

    # tbd: hack, ensure labels are strings  
    if @project.labeltrack
      @project.labeltrack.first.label.each do |l|
        l.instance_variable_set(:@title, l.title.to_s)
      end
    end
        
    # cache disabled -- must fix for narray
    #@cache_max_length_bytes = 1024*1024*8
    @cache_max_length_bytes = 0
    
    raise "No wavetracks were found in the project." if !wavetracks()
  end
  
  def v1_2?
    @is_version_1_2
  end
  
  def v1_3?
    !v1_2?
  end
  
  def bytes_per_element
    2
  end
  
  def cache(data, samples_range, desired_samples, cache_range)
    @data_cache_desired_samples_ratio = desired_samples / samples_range.length 
    @data_cache_range = cache_range
    @data_cache = data
  end
  
  def cached_data(samples_range)
    cache_intersection = @data_cache_range & samples_range
    cache_range = (cache_intersection.first - @data_cache_range.first...cache_intersection.last - @data_cache_range.first)
    # tbd: stereo
    [@data_cache[0][cache_range]]
  end
  
  def can_use_cache?(samples_range, desired_samples)
    @data_cache &&
      desired_samples / samples_range.length == @data_cache_desired_samples_ratio &&
      @data_cache_range & samples_range != nil
  end
  
  def channels
    wavetracks.length
  end
  
  def min_cache_samples
    seconds_to_samples(2)
  end
  
  def data_samples(samples_range, desired_samples)
  	if can_use_cache?(samples_range, desired_samples)
      return cached_data(samples_range) 
  	end
  	
  	warn { "uncached data request (#{desired_samples} samples, #{4 * desired_samples / 2} bytes)" }

  	data_size_bytes = desired_samples * bytes_per_sample()
    
    if data_size_bytes <= cache_max_length_bytes()
      raise('tbd: this code is not working as it should')
      desired_cache_samples = [min_cache_samples, desired_samples].max
      samples_cache_range = (samples_range.first...samples_range.first + desired_cache_samples)
      
      result = if v1_2?
        [@project.wavetrack[0].data(samples_cache_range, samples_cache_range.length)]
      else
        raise "implement me"
      end
        
	    cache(result, samples_range, desired_samples, samples_cache_range)
      cached_data(samples_range)
  	else
  		warn { "data too big for cache (#{data_size_bytes} > #{cache_max_length_bytes()})" }
      @data_cache = nil
      
      if v1_2?
        @project.wavetrack.collect { |wavetrack| wavetrack.data(file_handle_manager(), samples_range, desired_samples) }
      else
        @project.all_waveclips.collect { |waveclip| waveclip.data(file_handle_manager(), samples_range, desired_samples) }
      end
  	end
  end
  
  class SequentialChannelReader < SoundFile::SequentialReader
    def initialize(sound_file, sequence)
      super(sound_file)
      @waveblock_idx = 0
      @waveblock_iterator = 0
      
      @sequence = if sequence.kind_of?(Numeric)
        sound_file.sequences[sequence]
      else
        sequence
      end
      
      @waveblocks = @sequence.waveblock
      @file_handle_manager = sound_file.file_handle_manager
      @blockfile = @waveblocks[@waveblock_idx].simpleblockfile.first.block_file(@file_handle_manager, sound_file)
    end
    
    def read(samples)
      return nil if @iterator >= @sequence.num_samples()
      
      if samples == 0
        return NArray.sfloat(0)
      end
      
      samples = [@sequence.num_samples() - @iterator, samples].min
      result = NArray.sfloat(samples)
      
      write_idx = 0
        
      while samples > 0
        block_samples = [@blockfile.length_samples() - @waveblock_iterator, samples].min
        
        result[write_idx...write_idx + block_samples] = @blockfile.data_samples((@waveblock_iterator...@waveblock_iterator + block_samples), block_samples)
        
        write_idx += block_samples
        samples -= block_samples
        @iterator += block_samples
        @waveblock_iterator += block_samples
        
        if @waveblock_iterator >= @blockfile.length_samples()
          @waveblock_idx += 1
          @waveblock_iterator = 0
          
          if @waveblock_idx >= @waveblocks.length
            raise "Ran out of waveblocks before satisfying request (#{samples} remaining in request)." if samples > 0
            break
          end
           
          @blockfile = @waveblocks[@waveblock_idx].simpleblockfile.first.block_file(@file_handle_manager, sound_file)
        end
      end
  
      return result
    end
    
    def seek(sample)
      super
      @waveblock_idx = @sequence.waveblock_idx_for_sample(@iterator)
      @blockfile = @waveblocks[@waveblock_idx].simpleblockfile.first.block_file(@file_handle_manager, sound_file)
      @waveblock_iterator = sample - @waveblocks[@waveblock_idx].start
    end
  end
  
  class SequentialReader < SoundFile::SequentialReader
    def initialize(sound_file)
      super
      @readers = sound_file.sequences.collect { |sequence| SequentialChannelReader.new(sound_file, sequence) }
    end
    
    def read(samples)
      return nil if @iterator >= length_samples()
      
      samples = [@sound_file.length_samples() - @iterator, samples].min
        
      result = @readers.collect do |io| 
        channel_result = io.read(samples)
        
        if channel_result != nil
          channel_result
        else
          []
        end
      end
      
      @iterator += samples
      
      result
    end
    
    def seek(sample)
      super
      @readers.each { |io| io.seek(sample) }
    end
  end
  
  def sequential_reader
    SequentialReader.new(self)
  end
  
  def labels
    if !@project.labeltrack
      []
    else
      @project.labeltrack.first.label
    end
  end
  
  def length_samples
    if !@length
      max = sequence_lengths.max
      @length = wavetrack_to_project_samples(max[0], max[1])
    end
    
    @length
  end
  
  def moduleLogging_id
  	@moduleLogging_id
  end
  
  def path
  	origin()
  end
  
  def path_data
    @path_data ||= Path.to_directory(path().segments[0..-2] + [path().file.no_extension.literal + "_data"])
  end
  
  def sample_rate
    @project.rate
  end
  
  def sequences
    if v1_2?
      wavetracks().collect { |wavetrack| wavetrack.sequence.first }
    else
      
      result = []
        
      @project.all_waveclips.collect { |waveclip| waveclip.sequence.first }
    end
  end
  
  def sequence_lengths
    sequences().collect do |sequence| 
      [sequence.num_samples(), sequence.rate(self)]
    end
  end

  def wavetracks
    @project.wavetrack
  end
  
  def wavetrack_to_project_samples(samples, wavetrack_rate)
    samples * wavetrack_rate / @project.rate
  end
end
