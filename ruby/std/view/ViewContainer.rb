require 'std/PersistentWindow'
require 'std/notify'
require 'std/view/View'
require 'std/view/MultipleViewsTabbed'

module ViewContainers
  class ViewContainerWindow < PersistentWindow
    include Abstract
    include PlatformGUISpecific
    include ConfigYAML
    include NotifyHost
    
    attr_reader :target
    
    class NotifyViewDescriptionChange < Notify::Callback
    end
    
    def initialize(target, parent, viewClassBase = View, should_show = true)
      @target = if !target.viewable_kind_of?(AttributeReference::Base)
        AttributeReference::ObjectReadOnly.new(target)
      else
        target
      end
      
      @viewClassBase = viewClassBase
      @views = []

      super(parent, '', false)
      
      notify_listen(NotifyViewDescriptionChange, self) { updateTitle }
      updateTitle
      show if should_show
    end
  
    def configBasePath
      super + Path.new('Views')
    end
    
    def configElementName
      self.class.name + '_' + @views.first.class.name + '_' + @views.first.target().viewable_class.name 
    end
  
    def configElementNamePerObject
      configElementName + '_' + targetDescription()
    end
    
    def on_close
      @views.each { |view| view.view_destroy() }
      #GC.start
      destroy()
      #GC.start
    end
    
    def onViewableDependencyEvent(record)
    end
    
    def possibleViewClasses
      subclasses = @viewClassBase.subclasses
      subclasses << @viewClassBase if @viewClassBase != View
      target().views_possible(subclasses).uniq
    end
    
    def targetDescription
      return 'nil' if !target() 
      str = target().viewable_name.dup
      str << " (#{target().viewable_description})" if !target().viewable_description.empty?
      str
    end
  
    def updateTitle
      self.title = targetDescription()
    end
    
    def viewOfType(type)
      @views.each { |v| return v if v.kind_of?(type) }
    end
    
  protected
    def addView(v)
      @views << v
      v.viewable_dependant(self)
    end
	end
	
  # implements a single persistent frame window that shows the best view.
  class Single < ViewContainerWindow
    include PlatformGUISpecific
    
    def initialize(target, parent, viewClassBase = View, should_show = true)
      super
    end
    
    def makeClientArea
      scrolled_window = Wx::ScrolledWindow.new(self) do
        self.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
        set_scroll_rate(1, 1)
        self
      end
      
      def scrolled_window.layout
        super
        fit_inside()
        set_scroll_rate(1, 1)
      end
      
      scrolled_window
    end
    
    def make_controls
      super
      
      viewClasses = possibleViewClasses
      
      if viewClasses.empty?
        possible_info = if viewClasses.empty?
          'Nothing'
        else
          viewClasses.inspect
        end
        
        Kernel.raise "No suitable view class found for target #{target.class} (possible: #{possible_info}, supplied base: #{@viewClassBase})."
      end
      
      @view = viewClasses.first.new(target(), clientArea())
      addView(@view)
      clientArea().sizer.add(@view, 1, Wx::EXPAND)
    end

    def onViewableDependencyEvent(record)
      GUI.ensure_main_thread do
        clientArea().layout()
      end
    end
    
    def targetDescription
      "#{@view.description}: #{super}"
    end
    
    def view
    	@views[0]
    end
  end
  
  class Tabbed < ViewContainerWindow
    def make_controls
      super
      
      viewClasses = possibleViewClasses()

      Kernel.raise "No suitable view class found for target of type #{target().viewable_debuginfo}." if viewClasses.empty?
      
      @view = Views::MultipleViewsTabbed.new(target(), clientArea(), viewClasses)
      addView(@view)
      clientArea().sizer.add(@view, 1, Wx::EXPAND)
    end

    def targetDescription
      super
    end
  end
  
end
