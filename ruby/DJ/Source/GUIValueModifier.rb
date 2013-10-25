module GUIValueModifier
  module BaseCommon
    def min
      0.0
    end
    
    def max
      1.0
    end
    
    def targetDescription=(s)
    end
    
    def value
      0.5
    end

    def value=(v)
    end
    
    def displayValue(v)
    end
  end
  
	module BaseFox
    include BaseCommon

		def initialize(parent)
			if !self.packer
				@packer = FXPacker.new(parent)
				@packer.layoutHints = LAYOUT_FILL_X
				@packer.padBottom = 0
				@packer.padTop = 0
				@packer.vSpacing = 0
				@packer.create
			end
		end
		
		def packer
			nil
		end
	end
  
  class BaseWx < Wx::Panel
  	include PlatformGUISpecific
    include BaseCommon
  	
    def initialize(parent)
      super(parent)
      
      self.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
      self.size = [200, 100]
    	
    	initControls
    end
    
    def initControls
    	@horzTopSizer = Wx::BoxSizer.new(Wx::HORIZONTAL)
    	self.sizer.add(@horzTopSizer, 0, Wx::GROW)
    	
			@descLabel = Wx::StaticText.new(self, -1, '')
			@horzTopSizer.add(@descLabel, 1, Wx::GROW | Wx::ALIGN_LEFT)

			@descLabel.font = Wx::Font.new(8, Wx::FONTFAMILY_DEFAULT, Wx::FONTSTYLE_NORMAL, Wx::FONTWEIGHT_BOLD)
    end

    def targetDescription=(s)
    	@descLabel.label = s
    end
  end

  class SliderBase < Base
    def updateLabels
      updateValueText(self.showValue(self.value))
      updateMinText(self.showValue(self.min))
      updateMaxText(self.showValue(self.max))
    end
  
    def showValue(value)
      value.to_s
    end
  
    def alpha=(alpha)
      self.value = Mapping.linear(alpha, 0, 1, self.min, self.max)
    end
  
    def alpha
      return Mapping.linear(@slider.value, 0, 1, self.min, self.max)
    end
    
    # set absolute range of slider
    def setRange(min, max)
      PlatformGUI.specific
    end
    
    def value=(v)
      set_value(v, true)
    end
    
    def set_value(v, update_control=true)
      setSliderValue(v) if update_control
      updateValueText(self.showValue(self.value))
    end
    
    def value
      PlatformGUI.specific
    end
    
    # set slider to value in domain of [self.min, self.max]
    def setSliderValue(v)
      PlatformGUI.specific
    end

    # express current slider value in domain of [self.min, self.max]
    def sliderValue
      PlatformGUI.specific
    end

  protected
  
    def sliderMoved
      onSliderChanged
    end
  
    def sliderFinished
      onSliderChanged
    end
  
    def onSliderChanged
      self.set_value(self.sliderValue, false)
    end
    
    def updateValueText(text)
      PlatformGUI.specific
    end

    def updateMinText(text)
      PlatformGUI.specific
    end

    def updateMaxText(text)
      PlatformGUI.specific
    end
  end

  class SliderWx < SliderBase
    include PlatformGUISpecific
    
    @@sliderMin = -32768
    @@sliderMax = 32767

    def initControls
    	super
    	
    	sizer = self.sizer
    
    	horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
    	sizer.add(horz, 0, Wx::GROW)
    	
		  @labelMin = Wx::StaticText.new(self, -1, '')
    	horz.add(@labelMin)
      
      horz.add_stretch_spacer(1)
      
		  @labelValue = Wx::StaticText.new(self, -1, '')
    	horz.add(@labelValue)

      horz.add_stretch_spacer(1)

		  @labelMax = Wx::StaticText.new(self, -1, '')
    	horz.add(@labelMax)

    	@slider = Wx::Slider.new(self, -1, 0, 0, 0)
    	sizer.add(@slider, 0, Wx::GROW)
    	
      @slider.parent.evt_slider(@slider) do 
      	sliderMoved
    	end
    	
      #@slider.evt_scroll_lineup { sliderFinished }
      #@slider.evt_scroll_linedown { sliderFinished }
      #@slider.evt_scroll_pageup { sliderFinished }
      #@slider.evt_scroll_pagedown { sliderFinished }
      
      @slider.evt_scroll_thumbrelease do
        sliderFinished
      end
    end
    
    def setRange(min, max)
      @slider.set_range(@@sliderMin, @@sliderMax)
    end

    def updateValueText(text)
      @labelValue.label = text
      self.layout
    end

    def updateMinText(text)
      @labelMin.label = text
      self.layout
    end

    def updateMaxText(text)
      @labelMax.label = text
      self.layout
    end
    
    def setSliderValue(v)
      @slider.value = Mapping::linear(v, self.min, self.max, @@sliderMin, @@sliderMax).to_i
    end

    def sliderValue
      Mapping::linear(@slider.value, @@sliderMin, @@sliderMax, self.min, self.max)
    end
  end

	class SliderFox < SliderBase
    include PlatformGUISpecific
    
		def initialize(parent)
			super
			vertPacker = FXVerticalFrame.new(self.packer)
			vertPacker.layoutHints = LAYOUT_FILL_X | LAYOUT_FILL_Y
			vertPacker.padBottom = 0
			vertPacker.padTop = 0
			vertPacker.vSpacing = 0
			vertPacker.create
	
			horzPacker = FXHorizontalFrame.new(vertPacker)
			horzPacker.layoutHints = LAYOUT_FILL_X;
			horzPacker.create
	
			@valueLabel = FXLabel.new(horzPacker, "")
			@valueLabel.layoutHints = LAYOUT_CENTER_X | LAYOUT_TOP
			@valueLabel.create
	
			horzPacker = FXHorizontalFrame.new(vertPacker)
			horzPacker.layoutHints = LAYOUT_FILL_X
			horzPacker.create
			
			@labelMin = FXLabel.new(horzPacker, "")
			@labelMin.create
			
			@slider = FXRealSlider.new(horzPacker)
			@slider.layoutHints = LAYOUT_FILL_X
			@slider.create
			
			@labelMax = FXLabel.new(horzPacker, "")
			@labelMax.create

			@slider.connect(SEL_CHANGED) {
				sliderMoved
				false
			}
			
			@slider.connect(SEL_COMMAND) {
				sliderFinished
				false
			}
		end
	
		def updateMinText(text)
			@labelMin.setText(text)
		end
	
    def updateMaxText(text)
      @labelMax.setText(text)
    end
    
		def setRange(min, max)
			@slider.setRange(self.min, self.max)
		end
		
		def value
			@slider.value
		end
		
    def setSliderValue(v)
      @slider.value = Mapping.linear(v, self.min, self.max, @slider.getRange()[0], @slider.getRange()[1])
    end
    
    def sliderValue
      Mapping.linear(@slider.value, @slider.getRange()[0], @slider.getRange()[1], self.min, self.max)
    end

	protected
    def updateValueText(text)
      @valueLabel.text = text
    end
	end
  
  class CheckBase < Base
    def initialize(parent, title='', getProc=nil, setProc=nil)
    	@title = title
    	@getProc = getProc
    	@setProc = setProc
    	
    	super(parent)
    end
  end

  class CheckWx < CheckBase
		include PlatformGUISpecific

		def initControls
			super
			
			check = Wx::CheckBox.new(self, -1, @title)
			self.sizer.add(check, 0, Wx::ALL, 5)

      check.value = if @getProc
		    @getProc.call
			else
			  false
			end
			
			self.evt_checkbox(check) do
				@setProc.call(check.checked?) if @setProc
			end
		end
  end

	class CheckFox < CheckBase
		include PlatformGUISpecific
		
		def initControls
			super
			
			check = FXCheckButton.new(self, @title)
			check.create
			
			check.connect(SEL_COMMAND) { |sender, sel, checked| @setProc.call(checked) if @setProc }
			
			check.setCheck(@getProc.call) if @getProc
		end
	end
end
	
