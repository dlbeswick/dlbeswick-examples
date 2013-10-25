require 'std/EnumerableInstances'
require 'mutex_m'
require 'std/platform'

if PlatformRuby.v18?
  begin
  	require 'fastthread'
  rescue LoadError
  	puts 'SafeThread: fastthread not installed.'
  end
end

class SafeThread < Thread
	include EnumerableInstances

	attr_reader :instigator

	def self.exit
		Thread.current.exit(0)
	end

	def self.exit?
		if !Thread.current.kind_of?(SafeThread)
      false
    else
		  Thread.current.exit?
    end
  end

	def self.exitAll(timeout=2)
		puts "Terminating SafeThreads..."
		
		exitThreads = []
		
		self.instances.each do |t|
			# exitAll could be called from a SafeThread, so don't request exit if t is the current thread. 
			# unsure if this is necessary yet.
			#next if t == Thread.current 
			
			exitThreads << Thread.new do
				self.abort_on_exception = true
        t.exit(timeout)
			end
		end

		exitThreads.each do |t|
			t.join
		end

		puts "SafeThread.exit finished."
	end
	
  def self.loop(&block)
    thread = SafeThread.new do
      yield while !exit?
    end
    
    thread
  end
  
  def self.requestExit
    Thread.current.requestExit
  end

	def initialize(*args)
		self.abort_on_exception = true

    @safethread_exit_m = Mutex.new
		
    @instigator = caller[0..5].join("\n\t")
		
		begin
			super do
				yield
			end
		rescue Exception=>e
			puts "Exception during thread execution (#{e}.)"
			raise
		end
	end
	
	def exit?
		@safethread_exit_m.synchronize do
    	self[:safethread_exit]
    end
	end

	def request_exit
		@safethread_exit_m.synchronize do
	    self[:safethread_exit] = true
	  end
	end
  alias_method :requestExit, :request_exit

	def exit(timeout=2)
    return if !alive?
    
    if timeout == 0
      $stderr.puts "Terminating SafeThread #{self} immediately, by request."
      terminate()
    else
      puts "Waiting for SafeThread #{self}..."
      
  		requestExit()
  
      sleep(timeout)
      
      if alive?
        $stderr.puts "Thread unresponsive, forcing exit. Instigator: #{instigator()}"
        terminate()
      else
        puts "SafeThread #{self} exited."
      end
    end
	end
end