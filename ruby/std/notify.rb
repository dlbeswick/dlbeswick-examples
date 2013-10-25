module Notify
	class Base
	  attr_accessor :listener
	  
		def initialize(listener, &callback)
			@callback = callback
			@listener = listener
		end
		
		def onAdded(host)
		end
	end

	class Time < Notify::Base
		def initialize(listener, seconds, &callback)
		  super(listener, &callback)

			@seconds = seconds
			@lastAddTime = ::Time.now
		end
		
		def elapsed?
			return ::Time.now - @lastAddTime > @seconds
		end
	end
	
	class ItemAdd < Time
		def initialize(listener, seconds = 1, &callback)
			super(listener, &callback)
			
			@items = []
		end
		
		def onItemAdded(item, isLast)
			@items << item
			if isLast || elapsed?
				@callback.call(@items)
				@items.clear
			end
		end
	end
	
	class Callback < Notify::Base
		def call(*args)
			@callback.call(*args)
		end
	end
end

module NotifyHost
	def notify_listen(notify, listener=nil, &block)
		@notifies = [] if !@notifies

		if block != nil
			if !notify.kind_of?(Class)
				raise "'notify' must specify a class if called with a block."
			end
			
			notify = notify.new(listener, &block)
		end
	
		if !notify.listener()
      raise "A listener must be specified."
		end
		
		@notifies << notify
		notify.onAdded(self)
	end
	
:protected
	def notify_call(type, instigator, *args)
		raise "'type' must be kindof Notify::Callback (#{type}.)" if type.superclass != Notify::Callback
		notify_each(type) { |n| n.call(*args) if n.listener != instigator }
	end

	def notify_each(type)
		return if !@notifies
		@notifies.each do |n|
			yield n if n.kind_of?(type)
		end
	end
end
