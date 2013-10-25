module Resample
	class Base
		def self.resample(data, output_samples)
		end
	end

	class Point
		def self.run(data, output_samples)
			result = []
			
			output_samples.to_i.times do |i|
				result << data[Mapping.linear(i, 0, output_samples - 1, 0, data.length - 1)]
			end
			
			result
		end
	end

	class MinMax
		def self.run(data, output_samples)
      if output_samples == 1
        return [data.min, data.max]
      end
        
			result = []
			
			((output_samples.to_i) - 1).times do |i|
				start_range = Mapping.linear(i, 0, output_samples - 1, 0, data.length - 1)
				end_range = Mapping.linear(i + 1, 0, output_samples - 1, 0, data.length - 1) + 1
				slice = data[start_range...end_range]
				result << [slice.min, slice.max]
			end
			
			result
		end
	end
end