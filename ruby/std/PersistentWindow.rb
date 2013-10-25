require 'std/ConfigYAML'

module PersistentWindowBase
	include ConfigYAML
	include ModuleLogging
	
	persistent :saved_x
	persistent :saved_y
	persistent :saved_width
	persistent :saved_height

	def clientArea
		PlatformGUI.specific
	end
	
	def configBasePath
		super + Path.new('PersistentWindow')
	end
	
	# Restores the window position.
	# The per-object config file is attempted first, then the per-class file.
	def restoreWindow
		setDefaultMetrics
		
		begin
		  load(configPathPerObject().to_file)
		rescue DLB::Config::ExceptionLoad, SystemCallError
      begin
        load(configPath().to_file)
      rescue DLB::Config::ExceptionLoad, SystemCallError
        PersistentWindowBase.warn { "No config found at either #{configPathPerObject()} or #{configPath()}." }
      end
		end
		  
		restoreMetrics
	end
	
	def setDefaultMetrics
		@saved_x = 0
		@saved_y = 0
		@saved_width = 0
		@saved_height = 0
	end

	def restoreMetrics
		if outsideScreen?
			@saved_x = 0
			@saved_y = 0
		end
	end
	
	def outsideScreen?
		PlatformGUI.specific
	end
	
	def on_close
	end
	
	def onSave
		save
	end
	
	def onSavePerObject
    save(configPathPerObject())
	end
	
	def make_controls
		makeSave
	end
	
	def makeSave
		PlatformGUI.specific
	end

	def updateTitle
		PlatformGUI.specific
	end

:protected
	def validMetrics?
		@saved_width != nil && @saved_height != nil && @saved_x != nil && @saved_y != nil &&
		@saved_width != 0 && @saved_height != 0
	end
	
	def onShowWindow
		if !@restored
			restoreWindow
			@restored = true
		end
	end
end

if PlatformGUI.wx?
	class PersistentWindow < Wx::Frame
		include ConfigYAML
		include PersistentWindowBase
	
    def initialize(owner, title, showWindow=true)
			style = Wx::DEFAULT_DIALOG_STYLE & ~Wx::MINIMIZE_BOX
			style = style | Wx::RESIZE_BORDER | Wx::FRAME_NO_TASKBAR | Wx::FRAME_FLOAT_ON_PARENT
			
    	super(owner, -1, title, Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE, style)
    	
    	make_controls

    	evt_close do
    	  on_close
    	end
    	
			self.show if showWindow
    end

		def clientArea
			@clientArea
		end
		
    def makeClientArea
      Wx::Panel.new(self) do
        self.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
        self
      end
    end
    
		def make_controls
      @clientArea = makeClientArea  

      super
		end

		def outsideScreen?
			@saved_x > Wx::SystemSettings::get_metric(Wx::SYS_SCREEN_X)	|| @saved_y > Wx::SystemSettings::get_metric(Wx::SYS_SCREEN_Y);
		end
	
		def restoreMetrics
			super
			
			if validMetrics?
				move(Wx::Point.new(@saved_x, @saved_y))
				self.size = Wx::Size.new(@saved_width, @saved_height)
		  end
		end

		def show(shown=true)
			onShowWindow if shown
			super
		end

		def onSave
			@saved_x = self.position.x
			@saved_y = self.position.y
			@saved_width = self.size.x
			@saved_height = self.size.y
			super
		end
		
		def onSavePerObject
      @saved_x = self.position.x
      @saved_y = self.position.y
      @saved_width = self.size.x
      @saved_height = self.size.y
      super
		end
		
    def newSaveClassViewButton(parent)
      Wx::Button.new(parent, -1, "Save Pos (this view class)") do |b|
        evt_button(b) do
          onSave
        end
      end
    end
    
    def newSaveObjViewButton(parent)
      Wx::Button.new(parent, -1, "Save Pos (this object)") do |b|
        evt_button(b) do
          onSavePerObject
        end
      end
    end
    
		def makeSave
#			menu = Wx::Menu.new
#      menuBar = Wx::MenuBar.new
#			menuBar.append(menu, "Window Settings")
#			item = menu.append("Save Position")
#      self.menu_bar = menuBar
#	
#			evt_menu(item) do
#				onSave
#			end
		  horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
      clientArea().sizer.add(horz)  
 
      horz.add(newSaveClassViewButton(clientArea()))
      horz.add(newSaveObjViewButton(clientArea()))
		end
	end
end

if PlatformGUI.fox?
	class PersistentWindow < FXDialogBox
		include ConfigYAML
		include PersistentWindowBase
	
		def initialize(owner, title, opts=DECOR_ALL, x=50, y=50, width=320, height=240, padLeft=10, padRight=10, padTop=10, padBottom=10, hSpacing=4, vSpacing=4)
			@saved_x = x
			@saved_y = y
			@saved_width = width
			@saved_height = height
			
			load
			x = @saved_x
			y = @saved_y
			width =	@saved_width
			height = @saved_height
			
			super
	
			commonInitialize

			show
		end
		
		def makeSaveButton
			FXButton.new(self, "Save Pos") do |c|
				c.connect(SEL_COMMAND) {
					onSaveButton
				}
				c.create
			end
		end
	
	end
end