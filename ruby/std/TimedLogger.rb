class TimedLogger
	def initialize(periodSeconds, tag='', logInitial=false)
		@tag = tag
		@periodSeconds = periodSeconds
		
		if logInitial
			@timeLastLog = Time.at(0)
		else
			@timeLastLog = Time.now
		end
	end
	
	def log(io=$stdout, &messageBlock)
		if Time.now - @timeLastLog > @periodSeconds
			io.puts "[#{@tag}]: #{yield}"
			@timeLastLog = Time.now
			nil
		end
	end
end