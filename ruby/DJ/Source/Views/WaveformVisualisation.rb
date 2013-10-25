require './Source/Views/sourceview'

class WaveformVisualisationBase < SourceView
  include PlatformGUISpecific
	
  def initialize(target, parentWindow, updateFreq = 10.0, timeSpan = 6.0)
    super(target, parentWindow)

    source = target()

    @vis = Dsound::SoundVisualisationProgressive.new(source.visualisable, ($djApp.sampleRate * timeSpan).to_i, defaultWidth, defaultHeight)
  end
  
  def dirty?
    false
  end
  
  def defaultWidth
    300
  end
  
  def defaultHeight
    150
  end

  def description
    'Waveform'
  end

  def self.order
    -2
  end
  
  def self.suits_target?(target, target_class)
    target.respond_to?(:visualisable) && target.visualisable != nil
  end
end

class WaveformVisualisationWx < WaveformVisualisationBase
  include PlatformGUISpecific
  
  unenumerable # temporary
end

class WaveformVisualisationFox < WaveformVisualisationBase
  include PlatformGUISpecific
  
	def initialize(target, parentWindow, updateFreq = 10.0, timeSpan = 6.0)
		super(target, parentWindow)

		source = target

		@canvas = FXCanvas.new(self, nil, 0)
		@canvas.connect(SEL_PAINT) { paint; 1 }
		@canvas.create

		class << @canvas
			def getDefaultWidth
				300
			end
			
			def getDefaultHeight
				150
			end
		end

		@vis = Dsound::SoundVisualisationProgressive.new(source.visualisable, ($djApp.sampleRate * timeSpan).to_i, defaultWidth, @canvas.getDefaultHeight)

		@img = FXImage.new($djApp.fxApp, nil, IMAGE_OWNED | IMAGE_KEEP, defaultWidth, defaultHeight)
		@img.create

		@dc = FXDCWindow.new(@canvas)

		SafeThread.new {
			while true
				until @canvas.parent.shown?
					sleep 1.0
				end

				@canvas.update
				sleep(1.0 / updateFreq)
			end
		}
	end
	
	def paint
		@vis.draw
		@buf = @vis.ruby_buffer(@buf)
		@stream = FXMemoryStream.new if !@stream
		@stream.close
		@stream.open(FXStreamLoad,@buf)
		@img.loadPixels(@stream)
		@img.render

		@dc.begin(@canvas)
		@dc.drawImage(@img,0,0)
		@dc.end
	end
end
