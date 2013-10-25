require 'std/ConfigYAML'
require 'std/EnumerableSubclasses'
require 'std/notify'

class AudioLibrary
  include ConfigYAML
  include EnumerableSubclasses
	include NotifyHost
	attr_reader :initialised
	attr_reader :master_window_handle

	persistent :sources

	class NotifyInit < Notify::Callback
	end

	def initialize
		@sources = []
		@initialised = false
	end

	def addSource(source)
		@sources << source
	end

	def close
		Master.instances.each do |m|
		  puts "Stopping Master of type #{m.class}"
			m.close
		end
	end

	def initLibrary(masterWindow)
		@initialised = true
		@masterWindow = masterWindow
		@sources.each do |s|
			s.onLibraryInit(self)
		end
		notify_call(NotifyInit, self)
	end
	
  def master_window=(window_handle)
  end
  
	def devices
		raise "No Implementation."
	end
	
	def masterClass
    raise "No Implementation."
	end
	
	def monitorClass
    raise "No Implementation."
	end
	
	def mp3StreamClass
    raise "No Implementation."
	end
	
	def refreshDevices
    raise "No Implementation."
	end

	def sampleRate
		44100.0
	end
end
