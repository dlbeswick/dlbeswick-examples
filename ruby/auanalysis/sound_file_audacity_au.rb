require 'sound_file'
require 'std/resample'
require 'narray'
require 'mutex_m'

module SampleFormat
  class Base
    def bytes_per_sample
    end
    
    def sample_data_to_float(string_data)
    end
  end

  class Int16 < Base
    def bytes_per_sample
      2
    end
    
    def sample_data_to_float(string_data)
      # tbd: this code produces "object allocation during garbage collection" errors
      # during multithreaded export. Find out why. I suspect NArray mark/free code.
      @@mutex ||= Mutex.new
      @@mutex.synchronize do
      GC.disable
      
      factor = 1.0 / 32768.0
      
      result = NArray.to_na( string_data, NArray::SINT, string_data.length / bytes_per_sample() ).to_f * factor
      
      GC.enable
      result
      end
      
      #divisor_positive = 0x7FFF
      #divisor_zero_and_negative = 0x8000
      
      #narray = NArray.to_na( string_data, NArray::SINT, string_data.length / bytes_per_sample() )

      # find all values less than or equal to zero, and all values greater than zero, and apply
      # different divisors to each element depending on the result.
      #where_greater = narray.gt(0).to_f * divisor_positive 
      #where_lessequal = narray.le(0).to_f * divisor_zero_and_negative 
      
      #narray / (where_greater + where_lessequal)
      
      #string_data.unpack('s*').collect do |int|
      #  int * factor
      #end
    end
  end

  class Int24 < Base
    def bytes_per_sample
      3
    end
    
    def sample_data_to_float(string_data)
      raise "tbd"
      string_data.collect do |int|
        int / 16777215.0
      end
    end
  end

  class Float < Base
    def bytes_per_sample
      4
    end
    
    def unpack_encoding
      'f'
    end
    
    def sample_data_to_float(string_data)
      string_data.unpack('f*')
    end
  end
end

class AudacitySummaryFile
	include Raise
	
	@@header_tag = "AudacityBlockFile112"
	
	class SummaryRecords
		include Raise
		
		attr_reader :record_sample_size
		
		def initialize(full_range_samples, file_offset, record_sample_size)
			@parent_samples = full_range_samples
			@record_sample_size = record_sample_size
			@record_amount = [(full_range_samples / @record_sample_size).to_i, 1].max
			@file_offset = file_offset
		end
		
		def samples_rms(io, range_sound_file)
			file_data_rms(io)[sound_file_range_to_summary_range(range_sound_file)]
		end
		
		def samples_minmax(io, range_sound_file)
			result = file_data_minmax(io)[sound_file_range_to_summary_range(range_sound_file)]
      result
		end
		
		def suitable_for_range?(samples_range, desired_samples)
			samples_range.length > @record_sample_size && desired_samples <= samples_range.length / @record_sample_size 
		end
		
	protected
		def bytes_per_frame
			4 * elements_per_frame()
		end
    
		def elements_per_frame
		  3
		end
		
		def file_data(io)
			if !@file_data
				io.seek(@file_offset)
				read_bytes = @record_amount * bytes_per_frame()
		    @file_data = io.read(read_bytes).unpack('f*')
		    
		    records_read = @file_data.length / elements_per_frame()
		    
		    if @record_amount != records_read
		      raise "#{read_bytes} bytes were requested while reading summary of #{@record_amount} records, but #{records_read} records were read."
		    end
		  end
		  
		  @file_data
		end

		def file_data_rms(io)
			if !@file_data_rms
				file_data = file_data(io)
				
				@file_data_rms = []
				
	    	(0...file_data.length).step(3) do |i|
		    	@file_data_rms << file_data[i+2]
	    	end
	    end
	    
	    @file_data_rms
		end
		
		def file_data_minmax(io)
			if !@file_data_minmax
				file_data = file_data(io)
				
				@file_data_minmax = []
				
	    	(0...file_data.length).step(3) do |i|
		    	@file_data_minmax << file_data[i, 2]
	    	end
	    end
	    
	    @file_data_minmax
		end
		
		def sound_file_range_to_summary_range(range)
			first = Mapping.linear(range.first, 0, @parent_samples, 0, @record_amount)
			last = Mapping.linear(range.last, 0, @parent_samples, 0, @record_amount) 
			(first..last)  
		end
	end
	
	def initialize(file_offset, sound_file_audacity_parent)
		@file_offset = file_offset + @@header_tag.length
		@parent = sound_file_audacity_parent
		
		@records = []
		@records << SummaryRecords.new(length_samples(), offset_64K(), 65536) 
		@records << SummaryRecords.new(length_samples(), offset_256(), 256) 
	end
  
	def channels
	  1
	end
		
	def samples_minmax(io, range_sound_file, desired_samples)
		raise "Not enough samples exist to supply the request (#{frames_64K} < #{desired_samples})" if !suitable_for_range?(range_sound_file, desired_samples)
		
		record_for_range(range_sound_file, desired_samples).samples_minmax(io, range_sound_file)
	end
	
	def samples_rms(io, range_sound_file, desired_samples)
		raise "Not enough samples exist to supply the request (#{frames_64K} < #{desired_samples})" if !suitable_for_range?(range_sound_file, desired_samples)
		
		record_for_range(range_sound_file, desired_samples).samples_rms(io, range_sound_file)
	end
	
	def suitable_for_range?(samples_range, desired_samples)
		@records.find { |x| x.suitable_for_range?(samples_range, desired_samples) } != nil
	end
	
protected
	def bytes_per_frame
		4 * 3
	end
	
	def record_for_range(range_sound_file, desired_samples)
		result = @records.find { |x| x.suitable_for_range?(range_sound_file, desired_samples) }
		raise "No summary info handler for desired sample amount of '#{desired_samples}' over range '#{range_sound_file}'" if !result
		result
	end
	
	def length_samples
		@parent.length_samples
	end
	
	def frames_64K
		(length_samples().to_f / 65536).ceil
	end
	
	def offset_64K
		@file_offset
	end
	
	def offset_256
		offset_64K() + (frames_64K() * bytes_per_frame)
	end
end

class SoundFileAudacityAu < SoundFile
  include EnumerableSubclasses
  include Raise
  include Viewable
  
  def initialize(io, origin, sequence, block_file, sound_file_audacity)
    super(origin)
    
    @io = io
    
    encoding = encoding_header()
    
    magic = io.read(4)
    @data_offset = io.read(4).unpack(encoding).first
    @data_samples = block_file.len
    @sample_rate = sequence.rate(sound_file_audacity)
    
    @sample_format = case sequence.sampleformat
      when Sampleformat_int16 then SampleFormat::Int16.new
      when Sampleformat_int24 then SampleFormat::Int24.new
      when Sampleformat_float then SampleFormat::Float.new
    end
  end

  def au_header_size
    24
  end
  
  def summary
    @summary ||= AudacitySummaryFile.new(au_header_size(), self)
  end
    
  def reassign_io(io)
    @io = io
  end

  def bytes_per_element
    sample_format.bytes_per_sample()
  end
  
  def encoding_header
    'V'
  end

  def encoding_samples
  end

  def data_samples(samples_range, desired_samples = samples_range.length)
  	raise "Sample request range #{samples_range} is out of range of #{range_samples()}" if !range_samples().include_range?(samples_range) 

		if desired_samples != samples_range.length && summary().suitable_for_range?(samples_range, desired_samples)
			summary().samples_minmax(@io, samples_range, desired_samples)
		else
	    @io.seek(sample_to_file_pos(samples_range.first.to_i))
	    
	    length = samples_range.length.to_i
	      
	    result = NArray.sfloat(length)
	    output_idx = 0
	    
      while length > 0
        read_bytes = sample_format.bytes_per_sample() * [65536, length].min
        requested_samples = read_bytes / sample_format().bytes_per_sample()
        
        data = @io.read(read_bytes)
        raise "IO object returned 'nil' when trying to read #{read_bytes} bytes, sample range #{samples_range}, file length #{length_samples()} samples, io position #{@io.tell}" if !data
        
        converted = sample_format.sample_data_to_float(data)
        
        retrieved_samples = data.length / sample_format().bytes_per_sample()
        raise "#{requested_samples} samples requested but only #{retrieved_samples} were available." if requested_samples != retrieved_samples
        
        result[output_idx...output_idx + retrieved_samples] = converted
        
        output_idx += retrieved_samples

        length -= retrieved_samples
      end
	    
	    result
	  end
  end

  def data_result_is_summary?(samples_range, desired_samples)
    summary().suitable_for_range?(samples_range, desired_samples)
  end

  def sample_to_file_pos(sample)
    @data_offset + sample_format().bytes_per_sample() * sample
  end
  
  def length_samples
    @data_samples
  end
  
  def range_samples
  	(0...length_samples())
 	end
  
  def sample_format
    @sample_format
  end
  
  def sample_rate
    @sample_rate
  end
  
protected
   Sampleformat_int16 = 0x00020001
   Sampleformat_int24 = 0x00040001
   Sampleformat_float = 0x0004000F
end
