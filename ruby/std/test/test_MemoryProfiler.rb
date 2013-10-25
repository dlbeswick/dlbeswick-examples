require 'test/unit'
require 'MemoryProfiler'

class TC_MemoryProfiler < Test::Unit::TestCase
	class X
	end
	
	def test_recording
		GC.start

		start = MemoryProfiler.allocations

		MemoryProfiler.start
		
		X.new
		X.new
		X.new
		X.new

		GC.start
		MemoryProfiler.stop
		
		assert(MemoryProfiler.allocations == start, "Allocation record not removed (#{start} vs. #{MemoryProfiler.allocations})") 
	end
end
