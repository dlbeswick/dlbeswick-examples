module DNDData
  class Base
  	@@dndType = nil
  
  	attr_accessor :instigator
  
    if PlatformGUI.fox?
  	 def self.dndType(app)
  		 @@dndType = app.registerDragType('dnd_'+self.class.name) if !@@dndType
  		 @@dndType
  	 end
    end
  	
  	def draggable?
  		false
  	end
  	
  	def dropped(target)
  	end
  	
  	def description
  	end
    
    def sendToTarget(target)
      self.dropped(target)
      target.dndarea_onReceivedData(self)
    end
  end
end

module DNDData
  class Track < Base
    attr_accessor :track
    
    def initialize(track)
      @track = track
    end
  end
  
  class Object < Base
  	attr_accessor :object
  	
  	def initialize(object)
  		@object = object
  	end
  end
end