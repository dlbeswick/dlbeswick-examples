require 'std/view/graph_renderer'
require 'std/view/View'

class GraphViewBase < View
  include Views::AttributeReferenceView
  
  abstract
  
  attr_infer :view_domain_start do
    raise "Need at least one renderer, or set view_domain_start." if @renderers.empty?
    @renderers.first.domain(target()).first
  end
  
  attr_infer :view_domain_size do
    raise "Need at least one renderer, or set view_domain_start." if @renderers.empty?
    @renderers.first.domain(target()).last - @renderers.first.domain(target()).first
  end
  
  def self.suits_target?(target, target_class)
    target_class <= Array || target.respond_to?(:map_range)
  end
  
  def self.order
    -2.1
  end
  
	def initialize(target, parentWindow, renderers = nil)
  	@gridDivisions = 10
    
    @renderers = if renderers
      if !renderers.respond_to?(:each)
        [renderers]
      else
        renderers
      end
    else
      renderers_for(paintTarget(target))
    end
    
    super(target, parentWindow)
	end
	
	def add_renderer(renderer)
	  @renderers << renderer
    updateScrollDomain()
    updateSliderDomainSize()
	  updateViewDomain()
	end

	def domain
	  raise "No renderers defined." if @renderers.empty?
	  
	  result = [@renderers[0].domain(target()).normalise_inclusive.first, @renderers[0].domain(target()).normalise_inclusive.last]
	  
	  @renderers[1..-1].each do |renderer|
	    result[0] = [result[0], renderer.domain(target()).normalise_inclusive.first].min
      result[1] = [result[1], renderer.domain(target()).normalise_inclusive.last].max
	  end
	  
	  (result[0]..result[1])
	end
	
  def generateExecute
    redraw
  end

  def simple=(b)
    @simple = b
  end
  
  def showLabels=(b)
    PlatformGUI.specific
  end

  def domain=(range)
    raise "No renderers have been assigned." if !@renderers 
    @renderers.each { |r| r.domain = range }
    updateScrollDomain
  end
  
  def range=(range)
    raise "No renderers have been assigned." if !@renderers 
    @renderers.each { |r| r.range = range }
  end

  def range
    raise "No renderers defined." if @renderers.empty?
    
    result = [@renderers[0].range.first, @renderers[0].range.last]
    
    @renderers[1..-1].each do |renderer|
      result[0] = [result[0], renderer.range.normalise_inclusive.first].min
      result[1] = [result[1], renderer.range.normalise_inclusive.last].max
    end
    
    (result[0]..result[1])
  end

  def view_range=(range)
    raise "No renderers have been assigned." if !@renderers 
    @renderers.each { |r| r.view_range = range }
  end
  
  def view_domain_start=(value)
    @view_domain_start = value
    updateScrollDomain
    updateViewDomain
  end
  
  def view_domain_size=(value)
    @view_domain_size = value
		updateScrollDomain
		updateSliderDomainSize
    updateViewDomain
  end
  
  def renderer
    @renderers.first
  end
  
  def renderers
    @renderers.dup
  end
  
protected
	def renderers_for(target)
    if target.respond_to?(:map_range)
      if !target.respond_to?(:domain) || !target.respond_to?(:range)
        raise "An object supporting 'map_range' was supplied to GraphView without a renderer, but the object does not respond to 'domain' and 'range'."
      end
      
      [GraphRendererContinuous.new(target)]
    elsif target.viewable_kind_of?(Array)
	    [GraphRendererDiscrete.new { target } ]
	  else
	    raise "Unable to choose a renderer for target of class '#{target.class}'."
    end
	end
	
  def on_scroll_pageup 
    @view_domain_start = [view_domain_start() - view_domain_size(), domain().first].max
    updateViewDomain
  end
  
  def on_scroll_pagedown
    @view_domain_start = [view_domain_start() + view_domain_size(), domain().last].min
    updateViewDomain
  end
  
  def on_scroll_lineup
    @view_domain_start = [view_domain_start() - view_domain_size() * 0.05, domain().first].max
    updateViewDomain
  end
  
  def on_scroll_linedown
    @view_domain_start = [view_domain_start() + view_domain_size() * 0.05, domain().last].min
    updateViewDomain
  end
  
  def scrollDomainThumb
    PlatformGUI.specific
  end
  
  def sliderDomainSizeThumb
    PlatformGUI.specific
  end
  
  def onScrollDomain
    thumb = scrollDomainThumb()
    domain = domain()
    @view_domain_start = Mapping.linear(thumb, 0.0, 1.0, domain.first, domain.last - view_domain_size())
    updateViewDomain
  end
  
  def onSliderDomainSize
    thumb = sliderDomainSizeThumb()
    domain = domain()
    
    @view_domain_size = Mapping.linear(thumb, 0.0, 1.0, domain.first, domain.last)
    
    if view_domain_start() + view_domain_size() > renderer().domain(target()).last
			@view_domain_start = renderer().domain(target()).last - view_domain_size()
		end
		
    updateScrollDomain
    updateViewDomain
  end
  
  def updateViewDomain
  	changed = false

    first = view_domain_start()
    size = view_domain_size()
    
    @renderers.each do |renderer|
      domain = renderer.domain(target())

      if domain
        renderer_first = first
        renderer_last = renderer_first + size
      
        if renderer_first != renderer.view_domain(target()).first || renderer_last != renderer.view_domain(target()).last
          renderer.view_domain = (renderer_first...renderer_last)
          changed = true
        end
      else
        # if any renderer domain is undefined, then a redraw must occur.
        # the discrete renderer calculates domain and range on first draw, for instance. 
        renderer.view_domain = (start...start + size)
        changed = true
      end
    end
	    
	  if changed
	    generate()
	  end
  end
  
  def make_controls
    makeXMaxLabel if !simple?
    makeYMaxLabel if !simple?
    makeDrawingArea
    makeScrollDomain if !simple?
    makeSliderDomainSize if !simple?
    updateScrollDomain()
    updateSliderDomainSize()
    updateViewDomain()
  end
  
  def makeDrawingArea
    PlatformGUI.specific
  end
  
  def makeXMaxLabel
    PlatformGUI.specific
  end
  
  def makeYMaxLabel
    PlatformGUI.specific
  end
  
  def makeScrollDomain
    PlatformGUI.specific
  end
  
  def makeSliderDomainSize
    PlatformGUI.specific
  end
  
  def simple?
    false    
  end

  def updateScrollDomain
    PlatformGUI.specific
  end

  def updateSliderDomainSize
    PlatformGUI.specific
  end
  
  def paintTarget(target)
    target.get
  end
end

if PlatformGUI.fox?
	class GraphView < FXPacker
		include GraphViewBase

		def initialize(parent)
			super(parent)
			
			@xMax = FXLabel.new(self, '')
			@xMax.layoutHints = LAYOUT_SIDE_RIGHT | LAYOUT_SIDE_BOTTOM | LAYOUT_BOTTOM
			@yMax = FXLabel.new(self, '')
			@yMax.layoutHints = LAYOUT_SIDE_LEFT | LAYOUT_SIDE_TOP
			
			@canvas = FXCanvas.new(self) do |ctrl|
				ctrl.layoutHints = LAYOUT_FILL | LAYOUT_SIDE_RIGHT
				
				ctrl.connect(SEL_PAINT) do |sender, sel, event|
			    dc = FXDCWindow.new(ctrl, event)
			    doPaint(dc, points(dc, target))
			    dc.end
				end
				
				ctrl.enable
			end
			
			init_graphviewbase
		end
    
    def drawingAreaWidth
      @canvas.width
    end
    
    def drawingAreaHeight
      @canvas.height
    end
	
		def showLabels=(b)
			if b
				@xMax.show
				@yMax.show
			else
				@xMax.hide
				@yMax.hide
			end
		end
	
		def redraw
      updateLabels
      doDraw
			@canvas.update
		end
		
		def getDefaultWidth
			width
		end
	
		def getDefaultHeight
			height
		end
		
	protected
    def doDraw
      PlatformGUI.specific
    end
    
    def updateLabels
      @xMax.text = self.srcMax.to_s
      @yMax.text = self.max.to_s
    end

    def clearDC(dc)
      dc.setForeground(FXColor::White)
      dc.fillRectangle(0, 0, @canvas.width, @canvas.height)
    end
  
    def paintGrid(dc)
      dc.setForeground(FXRGB(200,200,200))
    
      points = self.gridDivisions
      points.times do |i|
        x = Mapping.linear(i, 0, points, 0, @canvas.width)
        dc.drawLine(x, 0, x, @canvas.height)
      end   
  
      points.times do |i|
        y = Mapping.linear(i, 0, points, 0, @canvas.height)
        dc.drawLine(0, y, @canvas.width, y)
      end   
    end
    
    def doPaint(dc)
      clearDC(dc)
      paintGrid(dc)
      
      return if self.min == self.max || @canvas.width == 0 || @canvas.height == 0
      
      dc.setForeground(FXColor::Black)
      
      points = []
      
      numPoints = self.segments
      numPoints.times do |i|
        current = map(i.to_f / (numPoints - 1))
        
        x = Mapping.linear(i.to_f, 0.0, numPoints.to_f - 1.0, 0.0, @canvas.width)
        y = Mapping.linear(current, self.min, self.max, @canvas.height, 0.0)
        
        y = 0 if !y.finite?
        y = Math.clamp(y, -32767, 32767)
  
        points << FXPoint.new(x.to_i, y.to_i)
      end
  
      dc.drawLines(points)
    end
  end
end

class GraphViewWx < GraphViewBase
  include PlatformGUISpecific

  def showLabels=(b)
  end
  
  def width=(v)
    self.set_size(Wx::Size.new(v, self.size.height))
    layout
  end
  
  def height=(v)
    self.set_size(Wx::Size.new(self.size.width, v))
    layout
  end
  
  def get_min_size
    Wx::Size.new(250, 250)
  end
  
protected
  def cached_rendering
    @cache_bitmap
  end
  
  def redraw
    @cache_view_domain_start = nil
    @drawingArea.refresh
    @drawingArea.update
  end
  
  def render_cache_valid?(dc)
    @cache_view_domain_start == @view_domain_start \
    && @cache_view_domain_size == @view_domain_size \
    && dc.size.x == @cache_width \
    && dc.size.y == @cache_height \
    && @cache_target_id == target().object_id
  end
  
  def save_cache_rendering(bitmap, dc)
    @cache_bitmap = bitmap
    @cache_target_id = target().object_id
    @cache_view_domain_start = @view_domain_start
    @cache_view_domain_size = @view_domain_size
    @cache_width = dc.size.x
    @cache_height = dc.size.y
  end

  def makeDrawingArea
    @drawingArea = Wx::Panel.new(self)
    
    # no need to erase background of drawing area
    @drawingArea.background_style = Wx::BG_STYLE_CUSTOM 
    
    layoutFillVertical(@drawingArea)
    
    @drawingArea.evt_size do
    	@drawingArea.refresh
    	@drawingArea.update
    end
    
    @drawingArea.evt_paint do
      @drawingArea.paint do |dc|
        if !render_cache_valid?(dc)
          # double-buffer
          bitmap = Wx::Bitmap.new(dc.size.x, dc.size.y)
          
          bitmap.draw do |memory_dc|
            each_height = size().y / @renderers.length
            
            @renderers.each_with_index do |r, i|
              r.doPaint(memory_dc, paintTarget(@target), i)
            end
          end
          
          save_cache_rendering(bitmap, dc)
        end
        
        dc.draw_bitmap(cached_rendering(), 0, 0, false)
      end
    end
  end

  def makeXMaxLabel
  end
  
  def makeYMaxLabel
  end

  def makeScrollDomain
    @scrollDomain = Wx::ScrollBar.new(self, -1)
    
    @scrollDomain.evt_scroll_thumbtrack { onScrollDomain }
    @scrollDomain.evt_scroll_pageup { on_scroll_pageup }
    @scrollDomain.evt_scroll_pagedown { on_scroll_pagedown }
    @scrollDomain.evt_scroll_lineup { on_scroll_lineup }
    @scrollDomain.evt_scroll_linedown { on_scroll_linedown }
    
    @textDomain = makeText(self, "")
    layoutAddHorizontal(@textDomain)
    layoutFillHorizontal(@scrollDomain)
    layoutNextRow()
  end
  
  def makeSliderDomainSize
    @sliderDomainSize = Wx::Slider.new(self, -1, 100, 1, 10000)
    
    evt_slider(@sliderDomainSize) { onSliderDomainSize }
    
    @textDomainSize = makeText(self, "")
    layoutAddHorizontal(@textDomainSize)
    layoutFillHorizontal(@sliderDomainSize)
  end
  
  def domain_page_size(domain)
    [Mapping.linear(view_domain_size() * 0.7, 0.0, domain.length, 0.0, scrollRange()), 1.0].max.to_i
  end
  
  def domain_per_pixel
  	view_domain_size() / @drawingArea.size.x
 	end
  
  def scrollRange
  	999999999
  end

  def scrollDomainThumb
    domain = domain()
    Mapping.linear(@scrollDomain.thumb_position, 0.0, scrollRange() - @scrollDomain.page_size, 0.0, 1.0)
  end
  
  def sliderDomainSizeThumb
    Mapping.power(@sliderDomainSize.value, 10, 1.0, 10000.0, 0.0, 1.0)
  end
  
  def updateScrollDomain
    domain = domain()
    thumb_position = Mapping.linear(view_domain_start(), domain.first, domain.last, 0.0, scrollRange()).to_i
    @scrollDomain.set_scrollbar(thumb_position, domain_page_size(domain), scrollRange(), domain_page_size(domain))
  end

  def updateViewDomain
  	super
    @textDomain.value = view_domain_start().to_s
    @textDomainSize.value = view_domain_size().to_s
  end

  def updateSliderDomainSize
    domain = @renderers[0].domain(target())
    return if !domain
      
    value = Mapping.power(view_domain_size(), 0.1, domain.first, domain.last, 0.0, 10000)
    @sliderDomainSize.value = value.to_i if !value.nan? # tbd: find out why nan happens
    @textDomainSize.value = view_domain_size().to_s
  end
end

class GraphViewDiscrete < GraphView
  def self.suits_target?(target, target_class)
    target.respond_to?(:discrete_range)
  end
  
protected
  def renderers_for(target)
    renderer = GraphRendererDiscrete.new(target) do |renderer, dc, target|
      target.discrete_range(renderer.view_domain(target()))
    end
    
    [renderer]
  end
end
