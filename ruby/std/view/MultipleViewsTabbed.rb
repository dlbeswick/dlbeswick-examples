require 'std/platform'
require 'std/view/View'

module Views
  class MultipleViewsTabbedBase < ViewBasicAttributeReference
    include Abstract
    include PlatformGUISpecific
    
    attr_accessor :views

    def initialize(target, parentWindow, viewClasses)
      Kernel.raise "No view classes given for target #{target.viewable_debuginfo}." if viewClasses.empty?
      @viewClasses = View.allPossibleFor(target, viewClasses)
      Kernel.raise "No view classes possible for target #{target.viewable_debuginfo}." if @viewClasses.empty?
      super(target, parentWindow)
    end

    def addTabForViewclass(c)
    end
    
    def initTabs
      @viewClasses.each do |c|
        addTabForViewclass(c)
      end
    end

    def make_controls
      super
      initTabs
    end
    
    def visible_view
      abstract
    end
  end

  class MultipleViewsTabbedWx < MultipleViewsTabbedBase
    include PlatformGUISpecific
    
    attr_reader :notebook
    
    unenumerable
    
    def visible_view
      @views[@notebook.selection]
    end
    
    def initTabs
      # only use a choicebook if there's more than one possible view class
      if @viewClasses.length > 1
        @notebook = Wx::Choicebook.new(self) do
          self.parent.sizer.add(self, 1, Wx::GROW)
        end
      end
      
      super
    end
    
    def makeSave
      # save button is added on every page, because the tab control cannot parent the button
    end
  
    def onViewableDependencyEvent(record)
      GUI.ensure_main_thread do
        # Need this code to force scrollbar calculations after layout changed
        if @notebook
          @notebook.current_page.fit_inside
        else
          @scrolledWindow.fit_inside
        end
        
        layout()
      end
      
      super
    end
    
    def scrolledWindowParent
      if @notebook
        @notebook
      else
        self
      end
    end
  
    def addTabForViewclass(c)
      @views ||= []
      
      @scrolledWindow = Wx::ScrolledWindow.new(scrolledWindowParent()) do |page|
        page.set_scroll_rate(1, 1)
        page.sizer = Wx::BoxSizer.new(Wx::VERTICAL)

        #page.sizer.add(newSaveButton(page))
        
        view = c.new(attribute(), page)
        page.sizer.add(view, 1, Wx::GROW)

        # depend on the view so that layout changes can be noticed and the scrollbars can be adjusted
        view.viewable_addDependant(self)

        @views << view
        
        if @notebook
          @notebook.add_page(page, view.description)
        else
          sizer().add(page, 1, Wx::GROW)
        end
      end
    end
  end

  class TabbedFox < MultipleViewsTabbedBase
    include PlatformGUISpecific

    def initTabs
      packer = FXVerticalFrame.new(@window,LAYOUT_FILL,0,0,0,0,0,0,0,0,0,0)
      packer.create
      @tabBar = FXTabBar.new(packer)
      @tabBar.connect(SEL_COMMAND) { |sender, sel, event| 
        @switcher.childAtIndex(@switcher.current).hide
        @switcher.setCurrent(event)
        @switcher.childAtIndex(@switcher.current).show
      }
      @tabBar.create
      
      @switcher = FXSwitcher.new(packer)
      @switcher.layoutHints = LAYOUT_FILL
      @switcher.create
      
      @tabBar.raiseWindow
      
      @switcher.childAtIndex(0).show if @switcher.numChildren > 0
      
      super
    end

    def addTabForViewclass(c)
      tab = FXTabItem.new(@tabBar, '')
      tab.create

      FXScrollWindow.new(@switcher) do |area|
        area.hide
        area.create
        packer = FXPacker.new(area, FRAME_RAISED)
        packer.layoutHints = LAYOUT_FILL
        packer.create

        addView(c.new(target, self, packer))
        tab.text = @views.last.description
      end
    end
  end
end