class MemoryProfiler
	def self.onAlloc(who)
		if @running
			@running = false
			@allocations[who.object_id] = [who.class, who.to_s[0..30]]
			@running = true
		end
	end
	
	def self.finalizeProc(id)
		proc do
			if @running
				@allocations.delete(id)
			end
		end
	end

	def self.allocations
		@allocations ||= {}
		@allocations.size
	end
	
	def self.start
		@allocations ||= {}
		@running = true
		
		def Object.new(*args)
			newObj = super
			
			ObjectSpace.define_finalizer(newObj, MemoryProfiler.finalizeProc(newObj.object_id))
			MemoryProfiler.onAlloc(newObj)
			
			newObj
		end
	end

	def self.stop
		GC.start
		@running = false
	end
	
	def self.reportByClass
		puts '------------------------'
		puts "Memory profile gc..."
		GC.start
		sleep(2)
		puts "Memory profiler by class:"
		if @allocations.empty?
			puts "All deallocated."
		else
			byClass = {}
			byClass.default = 0
			
			@allocations.each do |k, v|
				byClass[v[0]] += 1 
			end

			ary = byClass.to_a.sort do |a, b|
				b[1] <=> a[1]
			end
			
			ary.each do |i|
				puts "#{i[0]}: #{i[1]}"
			end
			
			puts
			puts "Strings: #{ObjectSpace.each_object(String) {}}"
			puts "#{@allocations.size} active objects."
		end
	end
	
	def self.reportBySize
		puts '------------------------'
		puts "Memory profile gc..."
		GC.start
		sleep(2)
		puts "Memory profiler by marshal size:"
		if @allocations.empty?
			puts "All deallocated."
		else
			byClass = {}
			byClass.default = 0
			
			@running = false
			@allocations.each do |k, v|
				s = ''
				fs = StringIO.new(s)
				begin
					Marshal.dump(ObjectSpace._id2ref(k), fs)
					fs.flush
					byClass[v[0]] += fs.pos
				rescue
				end
			end
			@running = true

			ary = byClass.to_a.sort do |a, b|
				b[1] <=> a[1]
			end
			
			ary.each do |i|
				puts "#{i[0]}: #{i[1]}"
			end
			
			puts
			puts "#{@allocations.size} active objects."
		end
	end
	
	def self.reportStrings
		puts '----------------------------'
		ObjectSpace.each_object(String) do |i|
			puts i
		end
	end
	
	def self.reportAll
		puts '------------------------'
		puts "Memory profile gc..."
		GC.start
		puts "Memory profiler report:"
		if @allocations.empty?
			puts "All deallocated."
		else
			puts "#{@allocations.size} active objects."
			@allocations.each do |k, v|
				puts "#{v[0]}: #{v[1]}"
			end
		end
	end
end