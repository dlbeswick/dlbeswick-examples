if PlatformGUI.fox?
	class MenuBarCategory
		attr_reader :pane
		attr_writer :onSelect
		
		def initialize(menuBar, title, &onInit)
			@pane = FXMenuPane.new(menuBar.fxObj.parent)
			@pane.create
			title = FXMenuTitle.new(menuBar.fxObj, title, nil, @pane)
			title.create
			
			title.connect(SEL_COMMAND) do
				@onSelect.call if @onSelect
			end
					
			onInit.call(self) if onInit

			menuBar.fxObj.layout
			menuBar.fxObj.parent.layout
		end
		
		def separate
			FXMenuSeparator.new(@pane).create	
		end
	end

	class MenuBarItem
		def initialize(menuBarCategory, text, &onSelect)
			FXMenuCommand.new(menuBarCategory.pane, text) do |ctrl|
				ctrl.connect(SEL_COMMAND) { onSelect.call }
				ctrl.create
			end
		end
	end

	class MenuBar
		attr_reader :fxObj
		
		def initialize(window, &onInit)
			
			@fxObj = FXMenuBar.new(window, LAYOUT_SIDE_TOP|LAYOUT_FILL_X)
			@fxObj.create
			
			onInit.call(self) if onInit
		end
	end

elsif PlatformGUI.wx?

	class MenuBarCategory
		attr_reader :menuBar
		attr_reader :menu
		attr_writer :onSelect
		
		def initialize(menuBar, title, &onInit)
			@menu = Wx::Menu.new 
			@menuBar = menuBar
			
      if PlatformOS.osx?
        puts "This is failing: fix"
      else
  			menuBar.menuBar.append(@menu, title)
      end

			onInit.call(self) if onInit
		end
		
		def separate
			self.menu.append_separator	
		end
	end

	class MenuBarItem
		def initialize(menuBarCategory, text, &onSelect)
			item = menuBarCategory.menu.append(text)
			menuBarCategory.menuBar.window.evt_menu item do
				onSelect.call if onSelect
			end
		end
	end

	class MenuBar
		attr_reader :menuBar
		attr_reader :window
		
		def initialize(window, &onInit)
			@window = window
			@menuBar = Wx::MenuBar.new
			
      if PlatformOS.osx?
        puts "This is failing: fix"
      else
        @window.menu_bar = @menuBar
      end
			
			onInit.call(self) if onInit
		end
	end
end