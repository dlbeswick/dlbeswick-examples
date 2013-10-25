require 'std/interval'
require 'std/mapping'

# tbd: data domain and range belongs in the data, not here
# the reason I did this is that GraphRenderer needs to be independent of the GraphView class, and
# should be able to be used with flat data (arrays), so there is no good central place for domain/range data.
# these issues are solvable, for instance, by incorporating domain/range methods in Array.
class GraphRendererBase
  include PlatformGUISpecific
  
  attr_accessor :gridDivisions

  def initialize
    @gridDivisions = 10
  end

  def domain(target)
    @domain
  end
    
  def range(target)
    @range
  end
  
  def domain=(range)
    @domain = range
  end

  def range=(range)
    @range = range
  end
  
  def view_domain=(range)
    @view_domain = range
  end
  
  def view_domain(target)
    if !@view_domain
      domain(target)
    else
      @view_domain
    end
  end
  
  def view_range=(range)
    @view_range = range
  end
  
  def view_range(target)
    if !@view_range
      range(target)
    else
      @view_range
    end
  end
  
  def drawingAreaWidth(dc)
    PlatformGUI.specific
  end

  def drawingAreaHeight(dc)
    PlatformGUI.specific
  end
  
  def doPaint(dc, target, order=0, data=data(dc, target))
    if order == 0
      clearDC(dc)
      paintGrid(dc) if gridDivisions() > 0
    end
    
    prepare_render_points(dc)
    
    if data && !data.empty? && drawingAreaWidth(dc) != 0 && drawingAreaHeight(dc) != 0
      render_points(dc, points(dc, data, target))
    end
  end

protected
  def clearDC(dc)
    PlatformGUI.specific
  end

  # retrieve samples of raw data for display, expressed in the target domain.
  def data(dc, target)
    []
  end

  # convert data from the target domain to screen space.
  def screen_points(dc, data, target)
    [[0, 0]]
  end
  
  # return screen space points appropriate to the target GUI platform.
  def points(dc, data, target)
    platform_points(dc, screen_points(dc, data, target))
  end
  
  # convert points to a format appropriate to the platform.
  def platform_points(dc, points)
    PlatformGUI.specific
  end
  
  def paintGrid(dc)
    PlatformGUI.specific
  end

  def prepare_render_points(dc)
    PlatformGUI.specific
  end
end

class GraphRendererWx < GraphRendererBase
  include PlatformGUISpecific
  
  def drawingAreaWidth(dc)
    # some dcs seems to return size as a Size, some as an array
    if dc.size.respond_to?(:[])
      dc.size[0]
    else
      dc.size.x
    end
  end

  def drawingAreaHeight(dc)
    # some dcs seems to return size as a Size, some as an array
    if dc.size.respond_to?(:[])
      dc.size[1]
    else
      dc.size.y
    end
  end

protected
  def clearDC(dc)
    dc.brush = Wx::WHITE_BRUSH
    dc.draw_rectangle(0, 0, drawingAreaWidth(dc), drawingAreaHeight(dc))
  end
  
  def paintGrid(dc)
    dc.pen = Wx::LIGHT_GREY_PEN
  
    width = self.drawingAreaWidth(dc)
    height = self.drawingAreaHeight(dc)
  
    points = self.gridDivisions
    points.times do |i|
      x = Mapping.linear(i, 0, points, 0, width)
      dc.draw_line(x, 0, x, height)
    end   

    points.times do |i|
      y = Mapping.linear(i, 0, points, 0, height)
      dc.draw_line(0, y, width, y)
    end   
  end
  
  def platform_points(dc, points)
    points.each do |p|
      p[0] = p[0].to_i
      
      if p[1].respond_to?(:finite?) && !p[1].finite?
        p[1] = 0
      else
        p[1] = Math.clamp(p[1], -32767, 32767).to_i
      end
    end
    
    points
  end

  def prepare_render_points(dc)
    dc.pen = Wx::BLACK_PEN
  end
  
  def render_points(dc, points)
    raise "Supply an implementation."
  end
end

class GraphRendererLineWx < GraphRenderer
  include PlatformGUISpecific
  
  def render_points(dc, points)
    (0...points.length-1).each do |i|
      dc.draw_line(points[i][0], points[i][1], points[i+1][0], points[i+1][1])
    end
  end
end

class GraphRendererDiscreteCommon < GraphRendererLine
  include PlatformGUISpecific

  attr_accessor :drawPoints

  # If the optional target is supplied, then domain and range values are taken from it,
  # otherwise those values must be supplied after construction. 
  def initialize(target=nil, &data_block)
    super()
    @drawPoints = true
    @domain = target.domain if target
    @range = target.range if target
    @data_block = data_block
    @target = target
  end
  
  def calculate_domain(dataX)
    self.domain = dataX.min..dataX.max
  end
  
  def calculate_range(dataY)
    self.range = dataY.min..dataY.max
  end
  
  def data(dc, target)
    if @data_block.arity == 3
      @data_block.call(self, dc, target)
    else
      @data_block.call
    end
  end

  def domain(data=data(nil, @target))
    if !@domain
      raise "No domain was assigned to the renderer, and no data from which it could be inferred was supplied." if !data
      @domain = calculate_domain(data.collect { |i| i[0] })
    end
  
    @domain
  end
  
  def range(data=data(nil, @target))
    if !@range
      raise "No range was assigned to the renderer, and no data from which it could be inferred was supplied." if !data
      @range = calculate_range(data.collect { |i| i[1] })
    end
  
    @range
  end
  
  def screen_points(dc, data, target)
    return [] if data.empty?
    
    domain = domain(data)
    range = range(data)
    
    drawingAreaWidth = drawingAreaWidth(dc)
    drawingAreaHeight = drawingAreaHeight(dc)
    view_domain = view_domain(target)
    view_range = view_range(target)
    
    result = []
      
    data.each do |d|
      # tbd: data points spanning the view boundaries will be lost 
      next if d[0] < view_domain.first && d[0] > view_domain.last
      next if d[1] < view_range.first && d[1] > view_range.last
      
      result << [
        Mapping.linear(d[0], view_domain.first, view_domain.last, 0.0, drawingAreaWidth),
        Mapping.linear(d[1], view_range.first, view_range.last, drawingAreaHeight, 0.0)
      ]
    end
    
    result
  end
end

class GraphRendererDiscreteWx < GraphRendererDiscreteCommon
  include PlatformGUISpecific
  
  def render_points(dc, points)
    super
    
    if drawPoints()
      points.each do |p|
        dc.draw_circle(p[0].to_i, p[1].to_i, 4)
      end
    end
  end
end

class GraphRendererContinuous < GraphRendererLine
  attr_writer :num_samples
  
  # If the optional target is supplied, then domain and range values are taken from it,
  # otherwise those values must be supplied after construction. 
  def initialize(target=nil, &block)
    super()
    @domain = target.domain if target
    @range = target.range if target
    @block = block if block
  end

  def data(dc, target)
    num_samples = num_samples(dc)
    
    result = sample_data(num_samples, target)
    
    postprocess_sampled_data(result, num_samples, drawingAreaWidth(dc))
  end
  
  # perform any resampling of sampled data, to suit renderer 
  def postprocess_sampled_data(data, desired_samples, drawing_area_width)
    if data.length > desired_samples
      resample_data(data, desired_samples)
    end
    
    data
  end
  
  def linear_mapping
    @mapping ||= Mapping::Linear.new
  end
  
  def num_samples(dc)
    if @num_samples.respond_to?(:call)
      @num_samples.call(dc)
    elsif @num_samples
      @num_samples
    else
      self.drawingAreaWidth(dc)
    end
  end
  
  def one_sample_per_pixel
    @num_samples = lambda { |dc| drawingAreaWidth(dc) }
  end
  
  def resample_data(data, num_samples)
    result = []
    
    linear_mapping = linear_mapping()
    
    num_samples.times do |i|
      map_domain_input = linear_mapping.calc_for(i.to_f, 0.0, num_samples.to_f - 1.0, 0.0, data.length.to_f - 1.0)
      result << data[map_domain_input]
    end
    
    result
  end

  def screen_points(dc, data, target)
    view_domain = view_domain(target)
    view_domain_ary = [view_domain.first, view_domain.last]
    
    view_range = view_range(target)
    view_range_ary = [view_range.first, view_range.last]
    
    domainMin, domainMax = view_domain_ary.min, view_domain_ary.max
    rangeMin, rangeMax = view_range_ary.min, view_range_ary.max
    
    drawingAreaWidth = drawingAreaWidth(dc)
    drawingAreaHeight = drawingAreaHeight(dc)

    linear_mapping = linear_mapping()
    
    points = []
    
    data.each_with_index do |current, i|
      next if !current 
      
      x = Mapping.linear(i.to_f, 0.0, (data.length - 1).to_f, 0.0, drawingAreaWidth).to_i
      y = linear_mapping.calc_for(current, rangeMin, rangeMax, drawingAreaHeight, 0.0)
      
      points << [x, y]
    end
    
    points
  end
  
protected
  def sample_data(num_samples, target)
    raise 'nil target' if !target
    
    domain = domain(target).normalise_inclusive
    view_domain = view_domain(target).normalise_inclusive
    view_domain_first = Math.clamp(view_domain.first, domain.first, domain.last)
    view_domain_last = Math.clamp(view_domain.last, domain.first, domain.last)
    view_domain = (view_domain_first..view_domain_last)
    
    if @block
      @block.call(self, view_domain, num_samples)
    elsif target.respond_to?(:map_range)
      target.map_range(view_domain, num_samples)
    else
      result = []

      linear_mapping = linear_mapping()
      
      num_samples.times do |i|
        map_domain_input = linear_mapping.calc_for(i.to_f, 0.0, num_samples - 1.0, view_domain.first, view_domain.last)
        result << target.map(map_domain_input, target)
      end
      
      result
    end
  end
end

# If more than 'minmax_sample_threshold' samples map to each x position, then vertical lines are drawn
# depicting the minimum and maximum extent of the data in the sampling interval. Otherwise, a standard
# linear interpolation is used to depict the sample data.
#
# Note: the kind_ofs in this code aren't pretty. The solution is probably to encapsulate the minmax renderer 
# entirely in its own class, and then create a third renderer that uses composition to switch between 
# continuous and minmax renderers.
class GraphRendererContinuousMinMaxWx < GraphRendererContinuous
  include PlatformGUISpecific
  
  attr_accessor :minmax_sample_threshold
  
  def initialize(target)
    super
    @minmax_sample_threshold = 4
    one_sample_per_pixel()
  end
  
  def data_is_minmax_format?(data)
    !data.empty? && data[0].viewable_kind_of?(Array)
  end
  
  def point_data_is_minmax_format?(points)
    points.first[1].viewable_kind_of?(Array)
  end
  
  def should_draw_minmax?(num_samples, drawing_area_width)
    samples_per_pixel(drawing_area_width, num_samples) > minmax_sample_threshold()
  end
  
  def postprocess_sampled_data(data, desired_samples, drawing_area_width)
    is_minmax = data_is_minmax_format?(data)
    
    if is_minmax || should_draw_minmax?(data.length, drawing_area_width)
      if is_minmax
        Resample::Point.run(data, drawing_area_width)
      else
        Resample::MinMax.run(data, drawing_area_width)
      end
    else
      data
    end
  end

  def samples_per_pixel(width, num_samples)
    num_samples.to_f / width
  end
  
  def screen_points(dc, data, target)
    if !data.empty? && data_is_minmax_format?(data)
      view_domain = view_domain(target)
      view_domain_ary = [view_domain.first, view_domain.last]
      
      view_range = view_range(target)
      view_range_ary = [view_range.first, view_range.last]
    
      domainMin, domainMax = view_domain_ary.min, view_domain_ary.max
      rangeMin, rangeMax = view_range_ary.min, view_range_ary.max
      
      drawingAreaWidth = drawingAreaWidth(dc)
      drawingAreaHeight = drawingAreaHeight(dc)
      
      points = []
      
      data.each_with_index do |current, i|
        x = Mapping.linear(i.to_f, 0.0, data.length.to_f - 1.0, 0.0, drawingAreaWidth).to_i
        y = [
          Mapping.linear(current[0], rangeMin, rangeMax, drawingAreaHeight, 0.0),
          Mapping.linear(current[1], rangeMin, rangeMax, drawingAreaHeight, 0.0)
        ]
        
        points << [x, y]
      end
      
      points
    else
      super
    end
  end

  def platform_points(dc, points)
    if !points.empty? && points.first[1].viewable_kind_of?(Array)
      points.each do |p|
        p[0] = p[0].to_i
        
        p[1].collect! do |pp|
          if pp.respond_to?(:finite?) && !pp.finite?
            0
          else
            Math.clamp(pp, -32767, 32767).to_i
          end
        end
      end
      
      points
    else
      super
    end
  end

  def render_points(dc, points)
    if !points.empty? && point_data_is_minmax_format?(points)
      center = drawingAreaHeight(dc) / 2
      points.each do |p|
        #dc.draw_line(p[0], p[1][0], p[0], p[1][1])
        p[1].min.upto(p[1].max) do |y|
          dc.draw_point(p[0], y)
        end
      end
    else
      super
    end
  end
end

class GraphRendererContinuousMinMax2D < GraphRendererContinuous
  attr_accessor :range_view_slice
  attr_reader :slice_renderer
  
  def initialize(target, range_view_slice=nil)
    super(target)
    
    @slice_renderer = GraphRendererContinuousMinMax.new(target)
    
    class << @slice_renderer
      attr_accessor :parent
      
      def view_range(target)
        parent.view_range(target)
      end
    end
    
    @slice_renderer.parent = self
    
    @range_view_slice = range_view_slice
  end
  
  def data(dc, target)
    num_samples = num_samples(dc)

    data = sample_data(num_samples, target)
    if !data.first.respond_to?(:[])
      raise "Data must be multi-dimensional, i.e. have elements that are arrays (first element was '#{data.first.class}', target is '#{target.class}'.)"
    end
    
    if range_view_slice()
      slice = ([range_view_slice().first, 0].max..[range_view_slice().last, data.length].min)
      data = data[slice]
    end
    
    postprocess_sampled_data(data, num_samples, drawingAreaWidth(dc))
  end
  
  def postprocess_sampled_data(data, desired_samples, drawing_area_width)
    data.collect do |array|
      slice_renderer().postprocess_sampled_data(array, desired_samples, drawing_area_width)
    end
  end
  
  def screen_points(dc, data, target)
    data.collect do |array|
      slice_renderer().screen_points(dc, array, target)
    end
  end

  def render_points(dc, points)
    points.collect do |array|
      slice_renderer().render_points(dc, array)
    end
  end
  
  def platform_points(dc, points)
    points.collect do |array|
      slice_renderer().platform_points(dc, array)
    end
  end  
  
  def render_points(dc, points)
    segments = points.length
    
    width = drawingAreaWidth(dc)
    height = drawingAreaHeight(dc)
    
    points.each_with_index do |vector, i|
      y0 = Mapping.linear(i, 0, segments, 0, height).to_i
      
      dc.set_device_origin(0, y0)
      dc.set_user_scale(1, 1.0 / segments)
      dc.destroy_clipping_region
      dc.set_clipping_region(0, 0, width, height)
      slice_renderer().render_points(dc, vector)
    end
  end
end

# Display 2D data, using colour intensity to render the second dimension.
# Colours should be specified as 3 element arrays giving RGB values.
class GraphRendererIntensity2DCommon < GraphRendererContinuous
  include PlatformGUISpecific
  
  attr_accessor :colour_min
  attr_accessor :colour_max

  def initialize(target, &block)
    super
    one_sample_per_pixel()
    @colour_min = [0, 0, 0]
    @colour_max = [1, 1, 1]
    @gridDivisions = 0
  end

  def clearDC(dc)
    # clearing is not necessary
  end
 
  def screen_points(dc, data, target)
    data
  end
end

class GraphRendererIntensity2DWx < GraphRendererIntensity2DCommon
  include PlatformGUISpecific

  def platform_points(dc, points)
    time = Time.now
    puts 'To image...'
    
    range_min = @range.first
    range_max = @range.last
    
    colour_mappings = @colour_min.zip(@colour_max).collect do |colours|
      Mapping::Linear.new(range_min, range_max, colours[0] * 255.0, colours[1] * 255.0)
    end
    
    drawingAreaWidth = drawingAreaWidth(dc) 
    
    images = []
    
    points.each do |p|
      image_data = ''
      
      if p.length > drawingAreaWidth
        p = Resample::MinMax.run(p, drawingAreaWidth)
        
        p.each do |intensity|
          image_data << [
            colour_mappings[0].calc(intensity[1]),
            colour_mappings[1].calc(intensity[1]),
            colour_mappings[2].calc(intensity[1])
            ].pack("C3")
        end
      else
        p.each do |intensity|
          image_data << [
            colour_mappings[0].calc(intensity),
            colour_mappings[1].calc(intensity),
            colour_mappings[2].calc(intensity)
            ].pack("C3")
        end
      end
      
      images << Wx::Image.new(p.length, 1)
      images.last.data = image_data
    end

    puts "Done. (#{Time.now - time} secs)"
    
    points = images
  end

  def render_points(dc, points)
#   drawingAreaWidth = drawingAreaWidth(dc)
#    (0...drawingAreaWidth).each do |x|
#     index = Mapping.linear(x.to_f, 0.0, drawingAreaWidth - 1.0, 0.0, points.length - 1.0)  
#     points[index][1].to_a.each_with_index do |intensity, y|
#       dc.draw_point(x,y)
#      end
#    end
    drawingAreaWidth = drawingAreaWidth(dc)
    drawingAreaHeight = drawingAreaHeight(dc)
    
    images = points.collect do |image|
      Wx::Bitmap.from_image(image.scale(drawingAreaWidth.to_i, 1))
    end
    
    (0...drawingAreaHeight).each do |y|
      idx = Mapping.linear(y, 0.0, drawingAreaHeight, 0.0, images.length)
      dc.draw_bitmap(images[idx], 0, y, false)
    end
  end
end

class GraphRendererMapping < GraphRendererContinuous
  def domain(target)
    (target.srcMin..target.srcMax)    
  end
  
  def range(target)
    (target.dstMin..target.dstMax)
  end

  def map(srcValue, target)
    target.calc(srcValue)
  end
end
