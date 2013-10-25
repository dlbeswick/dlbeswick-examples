require './Source/DNDData'
require 'std/EnumerableInstances'

class DragAndDropBase
  include PlatformGUISpecific
  
  def initialize(window)
    @window = window
    @dragging = false
  end
  
  def cleanup
  	DNDArea.instances.each do |i|
  		i.remove_instance
  	end
  	
  	@currentTarget = nil
  	
  	dragEnd
  end
  
  def currentDragData
  	@data
  end
  
  # call this function to drag data associated with a drag area
  def dragStart(dndArea)
    dragStartData(dndArea.dndarea_dndData)
  end

  # call this function to drag data that may not have a definite drag area as a source
  # (i.e., list items)
  def dragStartData(dndData)
    return if @dragging
      
    if !dndData.nil?
      @dragging = true
      @data = dndData
      tellDragStarting(@data)
    end
  end

  def dragEnd
  	return if !@dragging
  	
    @dragging = false
    
    if @currentTarget
      @data.sendToTarget(@currentTarget) if @currentTarget.dndarea_acceptsData?(@data)
    end
    
    tellDragEnding
    @data = nil
    @currentTarget = nil
  end

	# This function should be called whenever a left-mouse-up occurs in the application.
	# It returns true if the drag and drop instance processed the event.
	def onGlobalLeftMouseUp
		if @dragging
			dragEnd
			true
		else
			false
		end
	end

	def onMouseOverTarget(target)
		target.dndarea_showDataOver(@data)

   	if target != @currentTarget
   		target.dndarea_onDataOver(@data) if target
  	end
   	
   	@currentTarget = target
	end

	def onMouseOffTarget(target)
		target.dndarea_showIsAllowed(target.dndarea_acceptsData?(@data))
	end
	
protected
  def areaUnderCoords(coords)
    DNDArea.instances.each do |i|
      if i.dndarea_coordinateOver?(coords) 
        return i
      end
    end
    
    nil
  end

  def dragCursorAccept
    PlatformGUI.specific
  end
  
  def dragCursorMove
    PlatformGUI.specific
  end

  def dragCursorReject
    PlatformGUI.specific
  end

  def cursorPosition
    PlatformGUI.specific
  end
  
  def onCursorMotion(coords)
    newTarget = areaUnderCoords(coords)
    
    onMouseOffTarget(newTarget) if @currentTarget && newTarget != @currentTarget
    onMouseOverTarget(newTarget)
  end

  def overTarget?
    @currentTarget != nil
  end

  def tellDragStarting(data)
		DNDArea.instances.each do |i|
			i.dndarea_dragStarting(data)
		end
	end

  def tellDragEnding
		DNDArea.instances.each do |i|
			i.dndarea_dragEnding
		end
	end
end

if PlatformGUI.wx?
  class DataObjectRuby < Wx::DataObject
    attr_reader :obj
    def initialize(obj)
      @obj = obj
      super()
    end
    
    def get_all_formats(dir)
      [Wx::DF_TEXT]
    end
    
    def get_data_here(format)
      obj().to_s
    end
  
    def get_format_count(dir)
      case dir
        when Wx::DataObject::Get; 1
        when Wx::DataObject::Set; 1
      end
    end
    
    def set_data(format, len, buf)
      false
    end
  end
end

class DragAndDropWx < DragAndDropBase
  include PlatformGUISpecific
  
  attr_reader :defaultDropSource
  
  def initialize(window)
    super
    @defaultDropData = Wx::TextDataObject.new('hello')
    @defaultDropSource = Wx::DropSource.new(window, dragCursorAccept(), dragCursorMove(), dragCursorReject())
    @defaultDropSource.data = @defaultDropData 
  end
  
  def dragCursorAccept
    if PlatformOS.windows?
      Wx::Cursor.new(Wx::CURSOR_BULLSEYE)
    else
      return Wx::Icon.new('', 32, 32)
      Wx::Icon.from_bitmap(Wx::Cursor.new(Wx::CURSOR_BULLSEYE))
    end
  end
  
  def dragCursorMove
    if PlatformOS.windows?
      Wx::Cursor.new(Wx::CURSOR_HAND)
    else
      return Wx::Icon.new('', 32, 32)
      Wx::Icon.from_bitmap(Wx::Cursor.new(Wx::CURSOR_BULLSEYE))
    end        
  end

  def dragCursorReject
    if PlatformOS.windows?
      Wx::Cursor.new(Wx::CURSOR_NO_ENTRY)
    else
      return Wx::Icon.new('', 32, 32)
      Wx::Icon.from_bitmap(Wx::Cursor.new(Wx::CURSOR_BULLSEYE))
    end        
  end

  def dragEnd
  end
  
  def dragStartData(dndData)
    super
    
    defaultDropSource().data = DataObjectRuby.new(dndData)
    defaultDropSource().do_drag_drop
    
    tellDragEnding
    @dragging = false
  end
end

module DNDArea
  include EnumerableInstances
  
	attr_reader :dndarea_dndData
	attr_reader :dndarea_dndCtrl
	attr_reader :dndarea_dragAndDrop

	def self.inject(target, dragAndDropMgr, dndData=nil)
    raise "Supplied target is nil" if target == nil
    
    target.extend(DNDArea)
    
    target.dndarea_initialize(dragAndDropMgr, dndData)
    dndarea_onInject(target)
	end
  
	# Register an acceptance function for DNDData objects of class 'dndClass'.
	# Return true from the block to accept the DNDData object.  
	def dndarea_accept?(dndClass, &block)
		@dndarea_dndDataAcceptMappings[dndClass] = block
	end

	# Sets up handlers for dnd data classes to be dropped over this area. 
	def dndarea_accept(dndClass, &block)
		@dndarea_dndDataHandleMappings[dndClass] = block
	end

  def dndarea_initialize(dragAndDropMgr, dndData=nil)
    self.dndarea_dndData = dndData
    @dndarea_dndDataAcceptMappings = {}
    @dndarea_dndDataHandleMappings = {}
    @dndarea_dragAndDrop = dragAndDropMgr
  end
  
  def dndarea_onOwnDataDropped(data)
  end
	
  def dndarea_acceptsData?(data)
    if data
      if data.instigator != self
	   		@dndarea_dndDataAcceptMappings.each do |k,v|
					if dndarea_procMatchesDataClass?(data.class, k)
						if v.call(data) == true
				      return !respond_to?(:acceptsData?) || acceptsData?(data)
						end
					end
				end
			end
    end
    
    false
  end
  
  def dndarea_coordinateOver?(coords)
    PlatformGUI.specific
  end
  
	def dndarea_dndData=(data)
		raise "DND Data must be of type DNDData" if data != nil && !data.kind_of?(DNDData::Base) 
		@dndarea_dndData = data
		@dndarea_dndData.instigator = self if data
	end
	
  def dndarea_dragStarting(data)
  	dndarea_showIsAllowed(dndarea_acceptsData?(data))
  end
  
  def dndarea_dragEnding
  	dndarea_showNeutral
  end
  
  def dndarea_onDataOff(data)
  	dndarea_showIsAllowed(dndarea_acceptsData?(data))
  end

  def dndarea_onDataOver(data)
 		dndarea_showDataOver(data)
  end

	def dndarea_onReceivedData(data)
    dndarea_callProcsForDNDData(data)
	end
	
  def dndarea_showDataOver(data)
  	PlatformGUI.specific
  end
  
  def dndarea_showIsAllowed(allowed)
  	PlatformGUI.specific
  end

  def dndarea_showNeutral
  	PlatformGUI.specific
  end
  
	def dndarea_onLeftButtonUp
		@dndarea_dragAndDrop.dragEnd
	end
	
	def dndarea_onLeftButtonDown
		@dndarea_dragAndDrop.dragStart(@dndarea_dndData)
	end
	
protected
	def dndarea_procMatchesDataClass?(dndClass, procKeyClass)
		dndClass <= procKeyClass
	end
	
	def dndarea_callProcsForDNDData(dndData)
		@dndarea_dndDataHandleMappings.each do |k,v|
			if dndarea_procMatchesDataClass?(dndData.class, k)
				v.call(dndData)
			end
		end
	end
end

if PlatformGUI.fox?
	class DNDArea < FXPacker
		include DNDAreaBase
		
    @@app = nil
    
		def initialize(parent, dndData=nil, validDataBaseClass=nil)
			super(parent)

			dndareabase_init(parent, dndData, validDataBaseClass)
			
			create
			
      raise "No FXApp has been set" if !@@app
		
			self.dndCtrl = self
		end

		def activeCtrl
			@canvas
		end
		
    def self.app=(app)
      @@app = app
    end
    
	protected
		def doPaint(canvas, dc)
		end

    def dragStart
      super
      @app.beginWaitCursor
      @dndCtrl.grab
    end
    
    def dragEnd
      super
      @app.endWaitCursor
      @dndCtrl.ungrab
    end
  
    def isCursorOver(ctrl)
			ctrl.coordinateOver?(@app.getRootWindow.getCursorPosition)
		end

		def dndCtrl=(ctrl)
			@dndCtrl = ctrl

			@dndCtrl.enable

			@dndCtrl.connect(SEL_LEFTBUTTONPRESS) {
				onLeftButtonDown
			}

			@dndCtrl.connect(SEL_LEFTBUTTONRELEASE) {
				onLeftButtonRelease
			}
			
			@dndCtrl.connect(SEL_MOTION) { |sender, sel, event|
				onCursorMotion
			}		
		end

		def dragCursorAccept
			@@app.getDefaultCursor(DEF_SWATCH_CURSOR)
		end
		
    def dragCursorMove
      @@app.getDefaultCursor(DEF_SWATCH_CURSOR)
    end

    def dragCursorReject
			@@app.getDefaultCursor(DEF_DNDSTOP_CURSOR)
		end

		def setCursorAccept
			@@app.setWaitCursor(dragCursorAccept)
		end
		
		def setCursorReject
			@@app.setWaitCursor(dragCursorReject)
		end

		def acceptsData?(data)
			@dndCtrl.visible? && @dndCtrl.enabled? && super
		end

		def coordinateOver?(coords)
			screen = @dndCtrl.translateCoordinatesTo(@@app.getRootWindow, 0, 0)
			coords[0] >= screen[0] && coords[1] >= screen[1] && coords[0] <= (screen[0] + @dndCtrl.width) && coords[1] <= (screen[1] + @dndCtrl.height)
		end
	end

	class DNDAreaText < DNDArea
		def initialize(app, parent, text = '', dndData=nil, validDataBaseClass=nil, width=30, height=30)
			super(app, parent, dndData, validDataBaseClass)
			
			@defaultWidth = width
			@defaultHeight = height
			@text = text

			@canvas = FXCanvas.new(self)
			@canvas.layoutHints = LAYOUT_FILL
			@canvas.create

			@canvas.connect(SEL_PAINT) { |sender, sel, event|
			dc = FXDCWindow.new(@canvas, event)
			doPaint(@canvas, dc)
			dc.end
			}

			self.dndCtrl = @canvas

			self.dndCtrl.connect(SEL_RIGHTBUTTONPRESS) do
				text = self.toolTipText
				if text && !text.empty?
					FXToolTip.new(app, TOOLTIP_NORMAL, app.getRootWindow.getCursorPosition[0], app.getRootWindow.getCursorPosition[1]) do |ctrl|
						ctrl.setText(text)
						ctrl.create
						ctrl.show
					end
				end
			end
		end

		def toolTipText
			if self.dndData
				self.dndData.description
			else
				''
			end
		end

		def getDefaultWidth
			@defaultWidth
		end
			
		def getDefaultHeight
			@defaultHeight
		end

	protected
		def doPaint(canvas, dc)
			super
  		dc.background = FXColor::White
  		dc.foreground = FXColor::White
  		dc.fillRectangle(0, 0, canvas.width, canvas.height)
  
  		dc.background = FXColor::Black
  		dc.foreground = FXColor::Black
  	   
  		font = @@app.getNormalFont
  		w = font.getTextWidth(@text)
  		h = font.getTextHeight(@text)
  		
  		dc.setFont(@@app.getNormalFont)
  		dc.drawText((canvas.width - w) / 2, (canvas.height + h) / 2, @text)
		end
	end
end

if PlatformGUI.wx?
  module DNDArea
    include EnumerableInstancesWx
    
    class DNDAreaDropTarget < Wx::DropTarget
      attr_accessor :dndArea
      
      def initialize(dndArea)
        super(DataObjectRuby.new(dndArea.dndarea_dndData))
        @dndArea = dndArea
      end

      def on_enter(x, y, result)
        dnd = dndArea().dndarea_dragAndDrop
        dnd.onMouseOverTarget(dndArea())
        
        on_drag_over(x, y, result)
      end
      
      def on_drag_over(x, y, result)
        dnd = dndArea().dndarea_dragAndDrop
        
        if on_drop(x, y)
          Wx::DRAG_COPY
        else
          Wx::DRAG_NONE
        end
      end
      
      def on_data(x, y, result)
        dnd = dndArea().dndarea_dragAndDrop
        dnd.currentDragData.sendToTarget(dndArea())
        Wx::DRAG_COPY
      end
      
      def on_drop(x, y)
        dnd = dndArea().dndarea_dragAndDrop
        dndArea().dndarea_acceptsData?(dnd.currentDragData) == true
      end

      def on_leave
        dndArea().dndarea_dragAndDrop.onMouseOffTarget(dndArea())
      end
    end
  
    def self.dndarea_onInject(target)
      target.evt_left_down do
        target.dndarea_dragAndDrop.dragStart(target)
      end
    
      target.drop_target = DNDAreaDropTarget.new(target)
    end
  
    def dragCursorAccept
      dndarea_dragAndDrop.dragCursorAccept
    end

    def dragCursorMove
      dndarea_dragAndDrop.dragCursorMove
    end
  
    def dragCursorReject
      dndarea_dragAndDrop.dragCursorReject
    end
  
    def dndarea_coordinateOver?(coords)
      coords[0] >= self.screen_position.x && coords[1] >= self.screen_position.y &&
        coords[0] <= self.screen_position.x + self.size.width && coords[1] <= self.screen_position.y + self.size.height
    end

		def dndarea_dragStarting(data)
			@dndarea_originalColour = self.background_colour
      
	  	dndarea_showIsAllowed(dndarea_acceptsData?(data))
    end

		def dndarea_dragEnding
	  	dndarea_showNeutral()
		end

		def dndarea_showDataOver(data)
	  	if dndarea_acceptsData?(data)
  			self.background_colour = Wx::Colour.new(140, 255, 140)
		  	self.refresh
		  	self.update
  		end
		end
		
	  def dndarea_showIsAllowed(allowed)
	  	if allowed
  			self.background_colour = Wx::Colour.new(255, 255, 140)
  		else
	  		self.background_colour = Wx::Colour.new(255, 70, 70)
  		end
  		
	  	self.refresh
	  	self.update
	  end

	  def dndarea_showNeutral
	  	self.background_colour = @dndarea_originalColour if @dndarea_originalColour
	  	self.refresh
	  	self.update
	  end
  end
end
