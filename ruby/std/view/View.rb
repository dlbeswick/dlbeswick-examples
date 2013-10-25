require 'std/abstract'
require 'std/AttributeReference'
require 'std/ConfigYAML'
require 'std/deprecatable'
require 'std/EnumerableSubclasses'
require 'std/IncludableClassMethods'
require 'std/InheritableClassVariables'
require 'std/module_logging'
require 'std/notify'
require 'std/raise'
require 'std/SymbolDefaults'
require 'std/view/Viewable'
require 'drb'
require 'std/platform_gui'

if PlatformGUI.wx?
  require 'wx'
end

module ViewBase
	include Comparable
	include ConfigYAML
	include Deprecatable
	include IncludableClassMethods
	include ModuleLogging
	include NotifyHost
	include Raise
	
	# Note: views may no longer need to be viewable after the changes in wx::generateend.
	# Viewable is included for dependencies, which was once used to detect when child views need layout changes.
	include Viewable
  
  self.log=false

  attr_reader :target

	class NotifyDescriptionChange < Notify::Callback
	end
	
	class NoPossibleViewClassException < Exception
	end
	
	module ClassMethods
	  @included = proc do |mod|
      include InheritableClassVariables
      
      # tbd: use InheritableClassVariables.classmethods_attr_inherit
      attr_inherit :suits_kindof
    end
    
		def order
			0
		end
	
		def suits(class_or_name)
		  self.suits_kindof = class_or_name
		end
		
		def suits_target?(target, target_class)
		  if !@suits_kindof_class
  		  suits_kindof = suits_kindof()
  		  
  			raise "Please call the class method 'suits' or implement this function in your derived class (#{self}.)" if !suits_kindof
  			
  			@suits_kindof_class = case suits_kindof.class
  		    when String then eval(suits_kindof)
  		    when Module then suits_kindof
  		  end
  		  
  		  if !@suits_kindof_class.kind_of?(Module)
  		    raise "Bad 'suits' parameter for #{self}: 'suits #{suits_kindof.inspect}' evaluated to <#{@suits_kindof_class.inspect}>, must be a Module or Class."
  		  end
		  end
		  
      target_class <= @suits_kindof_class
		end

		# Allows base classes to do their own processing on suits_target before deferring to base classes if necessary.
		# AttributeReferenceView makes use of this, for example.
    def _suits_target?(target, target_class)
      suits_target?(target, target_class)
    end
     
		def addViewMapping(hashClassToView)
			@viewMappings ||= []
			@viewMappings.push(hashClassToView)
		end

		def beginViewMapping(hashClassToView, &block)
			addViewMapping(hashClassToView)
			result = block.call
			@viewMappings.pop
			result
		end

		def beginExcludeViewMapping(hashClassToView, &block)
			@viewExcludeMappings ||= []
			@viewExcludeMappings.push(hashClassToView)
			result = block.call
			@viewExcludeMappings.pop
			result
		end

		def viewMappingOverrides(target)
			results = []
	
			if @viewMappings			
				@viewMappings.each do |map|
					target.viewable_class.ancestors.each do |a|
						result = map[a]
						results.concat(Array(result)) if result
					end
				end
			end
			
			results
		end
	
		def viewMappingExclusions(target)
			results = []
	
			if @viewExcludeMappings
				@viewExcludeMappings.each do |map|
					target.viewable_class.ancestors.each do |a|
						result = map[a]
						results.concat(Array(result)) if result
					end
				end
			end
			
			results
		end

    def viewclass_possible_for(viewclass, target, target_class)
      return false if viewclass.abstract?
      
      if !(target_class <= Viewable)
        raise "'target' must include 'Viewable' in order to have views. (#{target.viewable_class})"
      end
    
      if viewMappingExclusions(target).include?(viewclass)
        ViewBase.log { "Target #{target.viewable_class} explicitly excludes view #{viewclass}" }
        return false 
      end

      if viewclass._suits_target?(target, target_class)
        ViewBase.log { "  Viewclass #{viewclass} is suitable (order #{viewclass.order})" }
        true
      else
        ViewBase.log { "  Unsuitable: #{viewclass}" }
        false
      end
    end
    protected :viewclass_possible_for
    
		def first_possible_for(target, subclasses_set=[])
      target_class = target.viewable_class
      
      # tbd: kind_of is bad, use respond_to? and call method to get 'operative' object
      ViewBase.log { "Finding first possible view classes for #{target.viewable_debuginfo}" }
		  if !(target_class <= AttributeReference::Base)
        if !(target_class <= Viewable)
          raise "'target' must include 'Viewable' in order to have views. (#{target.viewable_debuginfo})"
        end
		  end

      overrides = viewMappingOverrides(target)
      ViewBase.log { "View overrides active for #{target.viewable_class}\n#{viewMappingOverrides(target)}" if !overrides.empty? }
      
      # tbd: ask object for view mapping overrides (old comment -- still do?)
      attribute_overrides = viewMappingOverrides(target.get)  
      ViewBase.log { "View overrides active for AttributeReference's target #{target.get.class}\n#{viewMappingOverrides(target.get).inspect}" if target.kind_of?(AttributeReference::Base) && !attribute_overrides.empty? }

      (overrides + attribute_overrides).sort{ |a, b| b.order <=> a.order }.each do |c|
        return c if viewclass_possible_for(c, target, target_class)
      end
      
      subclasses_set.sort{ |a, b| b.order <=> a.order }.each do |c|
        return c if viewclass_possible_for(c, target, target_class)
      end

      raise NoPossibleViewClassException, "No possible view class for #{target.viewable_debuginfo}"
		end
		
		# Result is ordered by priority.
		def allPossibleFor(target, subclasses_set=subclasses())
		  # tbd: kind_of is bad, use respond_to? and call method to get 'operative' object
      if !target.viewable_kind_of?(AttributeReference::Base)
        target = AttributeReference::ObjectReadOnly.new(target)

        if !target.viewable_kind_of?(Viewable)
          raise "'target' must include 'Viewable' in order to have views. (#{target.viewable_class})"
        end
      else
        if !target.get.viewable_kind_of?(Viewable)
          raise "'target' must include 'Viewable' in order to have views. (#{target.viewable_class}, #{target.get.class})"
        end
      end
      
			ViewBase.log { "Finding all possible view classes for #{target.viewable_debuginfo}" }

      if !target.viewable_kind_of?(Viewable)
        raise "'target' must include 'Viewable' in order to have views. (#{target.viewable_debuginfo})"
      end
    
      ordered_possible = []
        
      overrides = viewMappingOverrides(target)
      ViewBase.log { "View overrides active for #{target.viewable_debuginfo}\n#{viewMappingOverrides(target)}" if !overrides.empty? }
      
      attribute_overrides = []
        
      # tbd: ask object for view mapping overrides (old comment -- still do?)
      attribute_overrides = viewMappingOverrides(target.get)  
      ViewBase.log { "View overrides active for AttributeReference's target #{target.get.class}\n#{viewMappingOverrides(target.get).inspect}" if target.viewable_kind_of?(AttributeReference::Base) && !attribute_overrides.empty? }

      ordered_possible.concat((overrides + attribute_overrides).sort{ |a, b| b.order <=> a.order })
        
      ordered_possible.concat(subclasses_set.sort{ |a, b| b.order <=> a.order }) 

      target_class = target.viewable_class
      ordered_possible.find_all { |c| viewclass_possible_for(c, target, target_class) }
		end

    def new_for_exact_target(target, parentWindow, subclasses_set=[])
      subclasses_set = Array(subclasses_set)
      
      ViewBase.log { "New view for #{target.viewable_class}" }
      ViewBase.log { "Attribute reference target is #{target.get.class}" } if target.viewable_kind_of?(AttributeReference::Base)
      
      viewClass = begin
        first_possible_for(target, subclasses_set)
      rescue NoPossibleViewClassException=>e
        raise "No suitable view found for target '#{target.viewable_debuginfo}'.\nGiven possibilities: #{subclasses.length} views, #{subclasses.compact.inspect}...\n"
      end

      ViewBase.log { "Chose viewclass #{viewClass}" }
      
      begin
        viewClass.new(target, parentWindow)
      rescue
        ViewBase.err { "Error creating view of type '#{viewClass}' for target #{target.viewable_debuginfo}" }
        raise
      end
    end
    
    # tbd: deprecated, don't use
		def newViewFor(target, parent_window, subclasses_set=subclasses())
      new_for_exact_target(target, parent_window, subclasses_set)
		end

		def new_for(target, parent_window, subclasses_set=subclasses())
      target = if !target.viewable_kind_of?(AttributeReference::Base)
        AttributeReference::ObjectReadOnly.new(target)
      else
        target
      end
      
      new_for_exact_target(target, parent_window, subclasses_set)
		end

		def viewAttribute(symbol, target, parentWindow, parentAttributeReference=nil, subclasses_set=subclasses())
			begin
				ViewBase.log { "Viewing attribute #{symbol} of #{target.viewable_class}" }
        
        attributeReference = if AttributeReference::Accessor.suitsSymbol?(target, symbol) 
          AttributeReference::Accessor.new(symbol, target, parentAttributeReference)
        else
          AttributeReference::Reader.new(symbol, target, parentAttributeReference)
        end
        
        target.viewable_on_attribute_view(attributeReference)
          
        defaultViewClass = target.viewable_defaultViewClassForSymbol(symbol)
        if defaultViewClass
          # If a default view class is available for the symbol, then choose the most appropriate view class
          # from the subclasses of that class.
			    new_for_exact_target(attributeReference, parentWindow, [defaultViewClass] + defaultViewClass.subclasses())
			  else
			    new_for_exact_target(attributeReference, parentWindow, subclasses_set)
        end
			rescue Exception=>e
        $stderr.puts "View.viewAttribute exception:"
        $stderr.puts e
        $stderr.puts e.backtrace
				#$stderr.puts "Error trying to view attribute: '#{target.viewable_class}.#{symbol}'"
				raise
			end
		end
		
    # in support of drb. Optional implementation currently provided by drbhacks.
    def send_authoritive(force_keep_result_reference, instance, symbol, *args)
      instance.send(symbol, *args)
    end
    
    # in support of drb. Optional implementation currently provided by drbhacks.
    def prevent_gc_authoritive(object)
    end
    
    # in support of drb. Optional implementation currently provided by drbhacks.
    def release_authoritive(instance)
    end
	end

	def initView(target, parent)
    # tbd: fix views so that 'generate' only happens once all initialisation is complete
	  # need a good place for a post-init step after ViewClass.new. wx?
		observe(target)
    
    # add parent view as a dependant, so that its layout can be updated when this view changes its structure.
    # this is important for some gui frameworks such as wx, but not for others such as fox.
    viewable_dependant(parent) if parent.respond_to?(:onViewableDependencyEvent)
	end

	def container	
    Algorithm.recurse(self, :parent) do |p|
      return p if p.kind_of?(ViewContainers::ViewContainerWindow)
    end
    
    raise "Couldn't find a ViewContainer in the view's hierarchy."
	end
	
	def description
		self.viewable_class.name
	end

	def ==(rhs)
    if rhs.kind_of?(ViewBase)
      self.target.viewable_name == rhs.target.viewable_name
    else
      #super
      false
    end
	end
	
	def observe(target, forceRegenerate=true)
    set_name("#{target.viewable_description} (#{self.class.name})")
    ViewBase.log { "#{self.class} is observing #{target.viewable_debuginfo}" }
    
		if target && !target.viewable_kind_of?(Viewable)
			Kernel.raise(ArgumentError, "Views can only observe objects of type 'Viewable' (#{target.viewable_class})")
		end
		
		if target != @target
			@target.viewable_remove_dependant(self) if @target && @target.viewable_supports_dependencies?

			@target = target

			if @target
        @target.viewable_dependant(self) if @target.viewable_supports_dependencies?
				
				if !@view_initialized
					@view_initialized = true
					initialized()
          
          make_controls()
          
					guiAdded()
				end
		
			  generate
			end
		else
			if !@view_initialized
				@view_initialized = true
				initialized
				make_controls
			end
		
			generate if forceRegenerate && @target
	  end
  
    clean
	end

	def viewDisengaged
		@target = nil
	end

	def destroyed?
		@destroyed && @destroyed == true
	end

	def onViewableDependencyEvent(record)
    return if @destroyed

    GUI.ensure_main_thread do
      # drb
      is_equal = if record.who.respond_to?(:drb_equal?)
        record.who.drb_equal?(target())
      else
        record.who.equal?(target())
      end
        
      if is_equal
        if dirty?
          ViewBase.log { "Dirty: view target #{@target.viewable_class} with summary #{@target.viewable_summary}." }
          generate
        else
          ViewBase.log { "Clean: view target #{@target.viewable_class} with summary #{@target.viewable_summary}." }
        end
      else
        # tbd: fix for drb (string conversion causes hang)
        #ViewBase.log { "The modified object is not the observed target (#{record.who} != #{@target})" }
        ViewBase.log { "  The modified object is not the observed target." }
  		end
      
      # a child view was modified, update layout 
      guiAdded()
      # propagate this notification to parents, so that layout will be calculated correctly. 
      modified(record.who)
    end
	end
  
protected
	# Subclasses will sometimes want to call super's make_controls after their own, so this function
	# can be used to ensure that all state necessary for gui creation is initialized.
	def initialized
  end

  def clean
    @dirtyComparison = if target() != nil
      target().viewable_dirtyTestReference
    else
      nil
    end
  
    verify = target() && target().viewable_clean_verify?
    if verify && dirty?
      puts 'old object'
      puts '----------'
      puts @dirtyComparison
      puts
      puts 'new object'
      puts '----------'
      puts target().viewable_dirtyTestReference()
      
      Kernel.raise "View '#{self.class}' with target object '#{target()}' (class #{target().class}): view still dirty after clean."
    end
  end

  def clean_verify?
    true
  end
  
  def cleanAncestors
    p = proc { |i|
      if i.respond_to?(:viewable_kind_of?) && i.viewable_kind_of?(View)
        i.clean
      end
      
      p.call(i.parent) if i.parent
    }
    
    p.call(self)
  end

  def view_destroy
    abstract
  end
  
  def destroyed?
    @destroyed == true
  end
  
  def view_destroy_children
    abstract
  end
  
  def dirty?
    if target().nil?
      !@dirtyComparison.nil?
    else
      !target().viewable_equivalent(@dirtyComparison)
    end
  end

	def generateBegin
	end
	
  # Initialize any controls here that change in response to the state of the view's target.
  def generateExecute
  end
  
	def generateEnd
    guiAdded()
	end
	
  # This function should be called after gui elements have been added.
  # Gui frameworks will override this function. For example, wx needs to call 'layout'.   
  def guiAdded
    PlatformGUI.specific
  end
  
  # call this when performing a large number of changes to a view.
  # guiAdded is called after the block.
  def guiUpdate(&block)
    yield
    guiAdded()
  end
  
  # Initialize any controls here that are permanent across the lifetime of the view.
	def make_controls
		# tbd: move subclasses' gui code out of generate/initialize and into this derived function 
		#PlatformGUI.specific
	end
	
	def generate
    ViewBase.log { "Generating #{self.class} #{object_id()} (parent: #{parent().class})." }
    if @destroyed
      ViewBase.log { "  View has been destroyed, aborting generate." }
      return
    end
    
    generateBegin
    generateExecute
    generateEnd
    ViewBase.log { "Generation done. (#{self.class} #{object_id()})" }
	end

	def descriptionChanged
		notify_call(NotifyDescriptionChange, self)
	end

	def repaint
	  PlatformGUI.specific
	end
	
	def viewbase_destroy
    ViewBase.log { "Destroying '#{self.class}' (#{self.object_id})" } 
		@destroyed = true
    @target = nil
    Viewable.remove_dependencies(self)
	end
end

# A View that makes use of OS GUI functionality.
module ViewGUI
protected
  # This function should be implemented such that an initial call begins a new vertical line, and subsequent 
  # calls arrange the children vertically in sequence.
	def layoutAddVertical(child=nil, padding=0, insertBefore=nil, &block)
		PlatformGUI.specific
	end

  # As for layoutAddVertical, but children fill all available horizontal space and an equal proportion of 
	# vertical space.
  def layoutFillVertical(child=nil, padding=0, insertBefore=nil, &block)
    PlatformGUI.specific
  end

  # As for layoutFillVertical, but children fill all available horizontal space.
	def layoutFillVerticalHorz(child=nil, padding=0, insertBefore=nil, &block)
		PlatformGUI.specific
	end
  
  # An initial call starts a new horizontal row. Subsequent calls arrange children horizontally within the 
	# current vertical row.
  def layoutAddHorizontal(child=nil, padding=0, &block)
    PlatformGUI.specific
  end
  
  def makeButton(parent, text, fit_to_text=false, &onClick)
    PlatformGUI.specific
  end

  def makeCheck(parent, text, state=false, &onChange)
    PlatformGUI.specific
  end

  def makeStaticText(parent, text)
    PlatformGUI.specific
  end

  def makeText(parent, text, field_width=nil, &onCommit)
    PlatformGUI.specific
  end
  
  def set_control_enabled(control, b)
    PlatformGUI.specific
  end
end

if PlatformGUI.wx?
	class View < Wx::Panel
		alias :wx_destroy :destroy
		
		include ViewBase
    include ViewGUI
	
		def initialize(target, parent)
      super(parent)

			makeSizer(target)
      
			initView(target, parent)
		end

		def generateBegin
		end
		
		def generateEnd
		  super

		  # tbd: use 'modified' call on view instead? maybe not, as there may be windows in the hierarchy that are not views.
		  if @first_generate_done == true && !destroyed?
		    Algorithm.recurse(self, :parent) do |p|
		      break if p.respond_to?(:destroyed?) && p.destroyed?
		      
		      p.layout if p.respond_to?(:layout)
		      
		      if p.respond_to?(:viewable_kind_of?)
		        break if p.viewable_kind_of?(ViewContainers::ViewContainerWindow)
		      else
            break if p.kind_of?(ViewContainers::ViewContainerWindow)
		      end
		    end
		  end
		  
      @first_generate_done = true
		end
	
		def makeSizer(target)
      self.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
		end

		def view_destroy
			if !destroyed?
        view_destroy_children(false)

        viewbase_destroy
        
        #window = Wx::Frame.new(GUI.main_window)
        #window.show
        #reparent(window)
        #window.close
        GUI.destroy_window(self)        
        #GC.start
        #hide()
			end
			
			true
	  end
	  
	  def destroy
	    view_destroy()
	    wx_destroy()
	  end
  
    def view_destroy_children(gui_destroy = true)
      destroyFunc = proc do |i|
        i.children.each do |w|
          destroyFunc.call(w)
        end
        
        if i.respond_to?(:viewbase_destroy)
          # wx does not delegate destroy to SWIG directors when destroying a control, so it's not possible to
          #  override the method to do cleanup. Instead, all child Views receive a viewbase_destroy call when
          #  the parent is destroyed.
          i.viewbase_destroy()
        end
        
        Viewable.remove_dependencies(i)
        
        GUI.destroy_window(i) if gui_destroy        
      end
      
      children().each do |w|
        destroyFunc.call(w)
      end
    end
	protected
    def guiAdded
      layout()
    end
    
    def guiUpdate(&block)
      freeze
      super
      thaw
    end
    
    def ensureSizer
      self.sizer ||= Wx::BoxSizer.new(Wx::VERTICAL)
    end
    
		def layoutAddVertical(child=nil, padding=0, insertBefore=nil, flags=Wx::TOP, proportion=0, &block)
      child = yield if !child
      
      ensureSizer()  
      @layoutHorzSizer = nil

      if !insertBefore
        sizer().add(child, proportion, flags, padding)
      else
        sizer().insert(insertBefore, child, proportion, flags, padding)
      end
      
      child
	  end
  
    def layoutFillVertical(child=nil, padding=0, insertBefore=nil, &block)
      layoutAddVertical(child, padding, insertBefore, Wx::TOP | Wx::EXPAND, 1, &block)
    end
  
    def layoutFillVerticalHorz(child=nil, padding=0, insertBefore=nil, &block)
      layoutAddVertical(child, padding, insertBefore, Wx::TOP | Wx::EXPAND, 0, &block)
    end
  
    def layoutFillHorizontal(child=nil, padding=0, &block)
    	layoutAddHorizontal(child, padding, 1, &block)
		end
		 
		def layoutNextRow
			@layoutHorzSizer = nil
   	end
   
    def layoutAddHorizontal(child=nil, padding=0, proportion=0, &block)
      child = yield if !child
      
      ensureSizer()
      
      if !@layoutHorzSizer 
        @layoutHorzSizer = Wx::BoxSizer.new(Wx::HORIZONTAL)  
        self.sizer.add(@layoutHorzSizer, 0, Wx::GROW)
      end
      
      @layoutHorzSizer.add(child, proportion, Wx::LEFT, padding)
      
      child
    end
    
    def makeButton(parent, text, fit_to_text=false, &onClick)
      style = if fit_to_text
        Wx::BU_EXACTFIT
      else
        0
      end
      
      Wx::Button.new(parent, -1, text, :style => style) do
        evt_button(self, &onClick)
      end
    end

    def makeCheck(parent, text, state=false, &onChange)
      Wx::CheckBox.new(parent, -1, text) do
        self.value = state
        
        evt_checkbox(self) do
          yield self.value if onChange
        end
      end
    end

    # onCommit is called with the text field value when enter is pressed.
    def makeText(parent, text, field_width=nil, &onCommit)
      make_text_with_char_events(parent, text, field_width, onCommit, nil)
    end
    
    # onEnterProc parameters: as for makeText
    # onCharProc parameters: key code, control down?, field value
    def make_text_with_char_events(parent, text, field_width, onEnterProc, onCharProc)
      size = if field_width
        extent = text_extent('W' * field_width)
        Wx::Size.new(extent[0], -1)
      else
        Wx::DEFAULT_SIZE
      end
      
    	Wx::TextCtrl.new(self, -1, text, Wx::DEFAULT_POSITION, size, Wx::TE_PROCESS_ENTER) do
        evt_char do |event|
          is_enter = event.key_code == Wx::K_RETURN || event.key_code == Wx::K_NUMPAD_ENTER
          
          # find out why this is needed. it seems likely that it's the windows code that was messed up.
          if PlatformOS.linux?
            is_enter = is_enter || event.key_code == 13
          end
          
          if is_enter
            if onEnterProc
              should_skip = onEnterProc.call(self.value)
              event.skip if should_skip
            else
              event.skip
            end
          else
            if onCharProc
              should_skip = onCharProc.call(event.key_code, event.control_down, self.value) == false
              event.skip if should_skip
            else
              event.skip
            end
          end
        end    	end
   	end
   
    def makeStaticText(parent, text)
      Wx::StaticText.new(parent, -1, text)
    end
    
    def repaint
      refresh()
    end
    
    def set_control_enabled(control, b)
      control.enable(b)
    end
	end
end

if PlatformGUI.fox?
	class View < FXPacker
		include ViewBase
		
		def initialize(target, parent)
			super(parent)
	
			setPadTop(0)
			setPadBottom(0)
			setPadLeft(0)
			setPadRight(0)
	
			create
	
			initView(target, parent)
		end
	
		def destroy
			wasDestroyed = @destroyed
			super
			
			self.app.rootWindow.setFocus
			
			self.parent.removeChild(self) if !wasDestroyed
		end
	end
end

# add common platform code
class View
	include EnumerableSubclasses
end

module Views
  include Deprecatable
  include ModuleLogging
  
	def self.basic_type?(object_or_class)
	  class_type = if object_or_class.class == Class
	    object_or_class
	  else
	    object_or_class.class
	  end
	  
    class_type <= Numeric \
      || class_type <= String \
      || class_type <= NilClass \
      || class_type <= TrueClass \
      || class_type <= FalseClass
	end

	def self.type_mutable?(object)
		object.viewable_kind_of?(NilClass) == false
	end

	def self.editable?(sourceClass)
		return sourceClass != Proc \
			&& sourceClass != NilClass
	end

	# Include this class in your views if they should deal with AttributeReferences rather than ordinary object
	# references.
	#
	# Derived classes should use 'attribute' to get the target AttributeReference object, and 'target' to get the
	# object reference that the AttributeReference contains. This enabled derived classes to use the 'target'
	# variable in a transparent manner.
	#
	# Derived classes should use 'suits_attribute?' to test the attribute reference for validity, and 'suits_target?'
	# to test the object reference as usual for view classes.
	#
	# TBD: mandate that AttributeReference objects should be used for all views? That would remove the need for
	# this distinction.
  module AttributeReferenceView
    include IncludableClassMethods
    
    def attribute
      @target
    end
    
    def target
      if @target
        # drb
        if @target.respond_to?(:get_authoritive)
          @target.get_authoritive
        else
          @target.get
        end
      else
        nil
      end
    end
    
    def observe(target, forceRegenerate=true)
      target_changed = !attribute() || !@previous_attribute_target || @previous_attribute_target != attribute().get
      
      @previous_attribute_target.viewable_remove_dependant(self) if @previous_attribute_target
      View.release_authoritive(@previous_attribute_target) if View.respond_to?(:release_authoritive)
      @previous_attribute_target = nil
    
      super(target, true)

      target().viewable_dependant(self) if target_changed && target() && target().viewable_supports_dependencies?
      @previous_attribute_target = target()
      
      begin
        View.prevent_gc_authoritive(target()) if View.respond_to?(:prevent_gc_authoritive)
      rescue
        err { "Error while calling prevent_gc_authoritive for target #{@previous_attribute_target.viewable_debuginfo}" }
        raise
      end
    end
    
    def onViewableDependencyEvent(record)
      return if @destroyed
      
      if record.who == attribute()
        GUI.ensure_main_thread do
          observe(attribute(), true)
          modified(self)
        end
        
        return
      end
      
      # tbd: add system to detect modifications to attribute references with common hosts and symbols.
      # currently each view could use different attribute objects. some changes will not be detected.
      # one solution is to use a common pool of attributereferences and return them through viewAttribute.
      # another solution is to make all attributereference clients listen to modifications to all attributereferences,
      # and then act if an AttributeSymbol has the same host and symbol.
      super
    end
    
    module ClassMethods
      def _suits_target?(target, target_class)
        target_class <= ::AttributeReference::Base && suits_attribute?(target) && suits_target?(target.get, target.get_target_class)
      end 
      
      def suits_attribute?(attribute)
        true
      end
    end
  end
  
  # tbd: remove? I think this class has been superceded by AllAttributes
#	class ObjectAttributeReference < View
#    include AttributeReferenceView 
#
#    def initialize(target, container, parentWindow)
#			@views = []
#			super
#		end
#		
#		def generateExecute
#			while !@views.empty?
#				@views.shift.destroy
#			end
#			
#			@views << View.newViewFor(@target.get, @container, self)
#			layoutAddVertical(@views.last)
#		end
#		
#		def self.suitsTarget?(target)
#			super && !Views.basicType?(target.get) && target.get.kind_of?(Viewable)
#		end
#		
#		def self.order
#			-3
#		end
#	end
	
  require 'std/view/array'
  
	class HashCommon < View
		include PlatformGUISpecific
    include AttributeReferenceView
    
    suits ::Hash

		def initialized
			@views = []
		end

		def make_controls
			makeTargetDescription(target().viewable_description)
		end
		
		def makeHashItem(keyView, valueView)
			PlatformGUI.specific
		end
		
		def makeTargetDescription(description)
			PlatformGUI.specific
		end
		
		def hashItemParent
			self
		end
		
		def onDeleteButton
			hashObject.delete(k)
			target().get.modified(self)
		end
		
		def generateExecute
			while !@views.empty?
				@views.shift.view_destroy
        #GC.start
			end
			
			hashObject = @target.get
		
			hashObject.each do |k, v|
				keyView = View.new_for(k, hashItemParent, possibleKeyViewSubclasses(k))
				  
				key_attribute = ::AttributeReference::HashKeyElement.new(k, @target.symbol, @target.host)
				valueView = View.newViewFor(::AttributeReference::HashValueElement.new(key_attribute, @target.host, @target), hashItemParent)
				
				@views << keyView
				@views << valueView
				
				makeHashItem(keyView, valueView) 
		  end
    
      guiAdded()
		end
		
  protected
    def possibleKeyViewSubclasses(key)
      View.subclasses
    end
  end
	
	class ObjectReferenceCommon < View
		abstract

    def labelTarget
      target()
    end

		def labelText
			obj = labelTarget()
			if obj.respond_to?(:viewableName) && obj.viewableName
				if !obj.viewableDescription.empty?
					"Obj reference: " + obj.viewableName + "\n#{obj.viewableDescription}"
				else
					"Obj reference: " + obj.viewableName
				end
			else
				"Obj reference: #{obj.class}"
			end
		end
	
		def self.order
			-10
		end

		def self.suits_target?(target, target_class)
			!(target_class <= ::AttributeReference::Base)
		end
	end
end

class ViewBasicAttributeReference < View
  include Views::AttributeReferenceView
  include Abstract
end

require "std/view/all_attributes"

# include gui platform-specific view code
require "std/view/View#{PlatformGUI.platform}"

# platform-independent views derived from platform-specific code classes are defined here 
module Views
  # A non-interactive last-resort class that indicates an attribute contains an object reference
  # and displays the text of its class name.
  class AttributeObjectReference < ObjectReference
    include AttributeReference
    
    def labelTarget
      target()
    end

    def self.order
      -10
    end
  end

  class HashObjectReferences < Hash
    def self.order
      -1
    end

  protected
    def possibleKeyViewSubclasses(elementObject)
      if [::Hash, ::String, ::Array, ::Numeric].any? { |c| elementObject.viewable_kind_of?(c) }
        super
      else
        View.subclasses(ObjectReference)
      end
    end
  end
end

require 'std/view/all_attributes_class_selector'
require 'std/view/AttributeReferenceTextField'
require 'std/view/attribute_object_reference_description'
require 'std/view/combo_choice'
require 'std/view/Date'
require 'std/view/inferred_attribute'
require 'std/view/GraphView'
require 'std/view/Range'
require 'std/view/two_state'
require 'std/view/unique_registry'
