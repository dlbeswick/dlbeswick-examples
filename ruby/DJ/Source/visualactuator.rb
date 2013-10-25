require 'std/math'
require './Source/actuator'
require 'std/DefaultCreateable'
require './Source/GUIValueModifier'
require './Source/Views/SourceModifierView'

# A VisualActuator is an Actuator presented to the user as a GUI element.
module VisualActuator
	module Actuator
		include ::Actuator

		def self.append_features(mod)
			super
	    mod.extend ClassMethods
		end
	
		module ClassMethods
			include EnumerableSubclasses
			
			unenumerable

			def createDefault(*args)
				raise "Visual actuators require a parent on initialization and so cannot be created via this function."
			end
		end
	
		attr_reader :dndArea
	
		def initVisualActuator(parent)
      @dndArea = self # tbd: remove references to dndarea
      DNDArea.inject(@dndArea, $djgui.dragAndDrop)
      
  		dndarea_accept?(DNDData::Object) do |data|
  			data.object.viewable_kind_of?(::Actuator)
  		end
  		dndarea_accept(DNDData::Object) do |data|
				data.object.actuator_connect(self.sourceModifier)
			end
			
			# fix: this is only necessary for visual actuators, should be fixed when they are data driven
			# this should be moved to UniqueIDConfig on id generation
			UniqueIDConfig.try_resolve_references
		end
		
		def initControls
			super

      if PlatformGUI.fox?
			 @dndArea.layoutHints = LAYOUT_FILL_X
			 @dndArea.padBottom = 0
			 @dndArea.padTop = 0
			 @dndArea.vSpacing = 0
      end
			
      if PlatformGUI.fox?
  			FXButton.new(@dndArea, 'src') do |ctrl|
  				ctrl.layoutHints = LAYOUT_SIDE_RIGHT
  				ctrl.create
  				
  				ctrl.connect(SEL_COMMAND) do
            onClickSrc
  				end
  			end
      else
        Wx::Button.new(self, -1, 'src', :style => Wx::BU_EXACTFIT) do |button|
          @horzTopSizer.add(button, 0, Wx::ALIGN_RIGHT)

  				button.parent.evt_button(button) do
            onClickSrc
  				end
        end
      end
		end
		
    def onClickSrc
      ViewContainers::Tabbed.new(self.sourceModifier, $djgui.masterWindow, SourceModifierView)
    end
	
		def actuator_connect(sourceModifier)
			super
			
			@dndArea.dndarea_dndData = DNDData::Object.new(self.sourceModifier)
			
			self.targetDescription=sourceModifier.viewable_name
		end

		def dndArea
			@dndArea
		end
	
		def value
			self.sourceValue
		end
		
		def set_value(v, update_control=true)
			self.sourceValue = v
			super
		end

		def showValue(value)
		  return value.to_s # xdrb
			self.sourceModifier.display(value)
		end

		def min
		  # tbd: this is an optimisation, clear on sourcemodifier min/max change
			@min ||= sourceModifier().min
			@min
		end
		
		def max
      # tbd: this is an optimisation, clear on sourcemodifier min/max change
      @max ||= sourceModifier().max
      @max
		end

		def updateFromSource(value)
		end
		
		def packer
			@dndArea
		end
    
		def title
		end
	end

	class Slider < GUIValueModifier::Slider
		include Actuator
		
		def initialize(parent, visualMin=nil, visualMax=nil)
      super(parent)

      initActuator(nil)
			initVisualActuator(parent)

			@visualMin = visualMin
			@visualMax = visualMax
		end

    def onReceivedDragAndDrop(data)
    	super
    end
    
		def actuator_connect(sourceModifier)
			setRange(sourceModifier.min, sourceModifier.max)
			super
		end
	
		def updateFromSource(value)
	    updateValueText(self.showValue(value))
	    setSliderValue(value)
		end
		
		def min
			if @visualMin
				@visualMin
			else
				super
			end
		end
		
		def max
			if @visualMax
				@visualMax
			else
				super
			end
		end
	end

  class TimebarBase < Slider
    def actuator_connect(sourceModifier)
      super

      self.sourceModifier.sources.each do |s|
        m = s.modifierOfType(SourceModifiers::Pitch)
        raise "Couldn't find pitch modifier, add pitch before visual timebar." if !m
        
        #xdrb: notify should change time amount on a pitch change, but causes hitches -- utilize pitch visual actuator instead or hook into bulk notify somehow (ideal)
        #@proc = proc { updateLabels }
        #m.notify_listen(SourceModifier::NotifyChange, self, &@proc) #xdrb 
      end

      dndarea_accept?(DNDData::Track) do
      	true
      end

      dndarea_accept(DNDData::Track) do |data|
        self.sourceModifier.sources[0].markUndo
        self.sourceModifier.sources[0].open(data.track)
      end

      @lastSliderValue = value
      reset
    end

    def update_on_source_modify?
      true#false
    end
    
    def updateFromSource(value)
      if !@dragging 
        super
        updateLabels
      end
    end

  protected
    def sliderMoved
      showValue(@slider.value)
      @lastSliderValue = @slider.value
    end
  
    def sliderFinished
      @slider.value = @lastSliderValue
      onSliderChanged
    end
  end

  class TimebarWx < TimebarBase
    include PlatformGUISpecific
  end
  
	class TimebarFox < TimebarBase
    include PlatformGUISpecific
    
		def initialize(parent)
			super(parent, 0.0, 1.0)
			
			@dragging = false

			@slider.connect(SEL_LEFTBUTTONPRESS) {
				@dragging = true
				false
			}

			@slider.connect(SEL_LEFTBUTTONRELEASE) {
				@dragging = false
				updateFromSource(value())
				false
			}
		end
	end
	
	class ShuttleBase < GUIValueModifier::Base
    include Actuator
    
    def initialize(parent)
      super
      initActuator(nil)
      initVisualActuator(parent)
      
      dndarea_accept?(DNDData::Track) do
      	true
      end

      dndarea_accept(DNDData::Track) do |data|
        self.sourceModifier.sources[0].markUndo
        self.sourceModifier.sources[0].open(data.track)
      end
		end
    
    def actuator_connect(sourceModifier)
      super
      raise "Shuttle sourceModifier is of type (#{self.sourceModifier.viewable_class}), but can only operate on Time" if !self.sourceModifier.viewable_kind_of?(SourceModifiers::Time)
    
      reset     
    end
    
    def reset
      onPause
    end

    def onCue
      self.sourceModifier.cue
      onModifiedSource
    end

    def onCueSet
      self.sourceModifier.cuePos = self.source.position
    end

    def onRewind
      self.value=0
    end

    def onBack
      self.sourceModifier.cuePos = Math.max(0, self.sourceModifier.cuePos - (50.0 / self.source.length))
      self.sourceModifier.cue
      onModifiedSource
    end
    
    def onFwd
      self.sourceModifier.cuePos = Math.min(self.source.length, self.sourceModifier.cuePos + (50.0 / self.source.length))
      self.sourceModifier.cue
      onModifiedSource
    end

    def onPlay
      self.source.play
      updateFromSource(value())
    end

    def onPause
      self.source.pause
      updateFromSource(value())
    end
    
  protected
    def source
      self.sourceModifier.sources[0]
    end
  end
  
  class ShuttleWx < ShuttleBase
    include PlatformGUISpecific

    def initControls
			super

			sizer = Wx::BoxSizer.new(Wx::HORIZONTAL)
			self.sizer.add(sizer, 1, Wx::GROW)

			@rewind = Wx::Button.new(self, -1, "|<<", :style => Wx::BU_EXACTFIT)
			sizer.add(@rewind)
			@cue = Wx::Button.new(self, -1, "Cue", :style => Wx::BU_EXACTFIT)
			sizer.add(@cue)
			@back = Wx::Button.new(self, -1, "<<", :style => Wx::BU_EXACTFIT)
			sizer.add(@back)
			@fwd = Wx::Button.new(self, -1, ">>", :style => Wx::BU_EXACTFIT)
			sizer.add(@fwd)
			@cueset = Wx::Button.new(self, -1, "Set Cue", :style => Wx::BU_EXACTFIT)
			sizer.add(@cueset)

			@play = Wx::Button.new(self, -1, ">", :style => Wx::BU_EXACTFIT)
			sizer.add(@play)
			@pause = Wx::Button.new(self, -1, "||", :style => Wx::BU_EXACTFIT)
			sizer.add(@pause)

			@cue.parent.evt_button(@cue) {
        onCue
			}

			@cueset.parent.evt_button(@cueset) {
        onCueSet
			}

			@rewind.parent.evt_button(@rewind) {
        onRewind
			}

			@back.parent.evt_button(@back) {
        onBack
			}
			
			@fwd.parent.evt_button(@fwd) {
        onFwd
			}

			@play.parent.evt_button(@play) {
        onPlay
			}

			@pause.parent.evt_button(@pause) {
        onPause
			}
		end
		
		def updateFromSource(value)
			super
			
			if self.source.playing
				@play.hide
				@pause.show
			else
				@play.show
				@pause.hide
			end
			
			@play.parent.layout
		end
  end
  
	class ShuttleFox < ShuttleBase
    include PlatformGUISpecific
		
		def reset
			@switcher.current = 0
			super
		end
		
		def initialize(parent)
			super

			horzPacker = FXHorizontalFrame.new(self.packer)
			horzPacker.layoutHints = LAYOUT_FILL_X;
			horzPacker.create

			@rewind = FXButton.new(horzPacker, "|<<")
			@rewind.create
			@cue = FXButton.new(horzPacker, "Cue")
			@cue.create
			@back = FXButton.new(horzPacker, "<<")
			@back.create
			@fwd = FXButton.new(horzPacker, ">>")
			@fwd.create
			@cueset = FXButton.new(horzPacker, "Set Cue")
			@cueset.create

			@switcher = FXSwitcher.new(horzPacker)
			@switcher.create
			@play = FXButton.new(@switcher, ">")
			@play.create
			@pause = FXButton.new(@switcher, "||")
			@pause.create

			@cue.connect(SEL_COMMAND) {
        onCue
			}

			@cueset.connect(SEL_COMMAND) {
        onCueSet
			}

			@rewind.connect(SEL_COMMAND) {
        onRewind
			}

			@back.connect(SEL_COMMAND) {
        onBack
			}
			
			@fwd.connect(SEL_COMMAND) {
        onFwd
			}

			@play.connect(SEL_COMMAND) {
        onPlay
			}

			@pause.connect(SEL_COMMAND) {
        onPause
			}
		end
		
		def updateFromSource(value)
			super
			
			if self.source.playing
				@switcher.current = 1
			else
				@switcher.current = 0
			end
		end
	end
	
  class PitchShuttleBase < GUIValueModifier::Base
    include Actuator

    def initialize(parent)
      super(parent)
      initActuator(nil)
      initVisualActuator(parent)

      @bigIncrement = 2.0
      @smallIncrement = 0.25
      @bigIncrementPerm = 0.1
      @smallIncrementPerm = 0.01

      setup('<<', -@bigIncrement)
      setup('<', -@smallIncrement)
      setup('>', @smallIncrement)
      setup('>>', @bigIncrement)
    end
    
    def setup(text, increment, permanent = false)
      PlatformGUI.specific
    end
  end
	
  class PitchShuttleWx < PitchShuttleBase
    include PlatformGUISpecific

    def initControls
      super
      @sizer = Wx::BoxSizer.new(Wx::HORIZONTAL)
      self.sizer.add(@sizer, 1, Wx::GROW)
    end
    
    def setup(text, increment, permanent = false)
      button = Wx::Button.new(self, -1, text, :style => Wx::BU_EXACTFIT)
      @sizer.add(button)
      if !permanent
        button.evt_left_down() do
          self.value = value + increment
        end
        button.evt_left_up() do
          self.value = value - increment
        end
      end
    end
  end
  
	class PitchShuttleFox < PitchShuttleBase
    include PlatformGUISpecific
    
		def initialize(parent)
			super(parent)
			
			@horzPacker = FXHorizontalFrame.new(self.packer)
			@horzPacker.layoutHints = LAYOUT_FILL_X;
			@horzPacker.create
						
			button = FXButton.new(@horzPacker, "0")
			button.connect(SEL_COMMAND) {
				self.value=100.0
			}
			button.create
		end
		
		def setup(text, increment, permanent = false)
			button = FXButton.new(@horzPacker, text)
			if (!permanent)
				button.setFont($djApp.fontBold)
				button.connect(SEL_LEFTBUTTONPRESS) {
					self.value = value + increment
					false
				}
				button.connect(SEL_LEFTBUTTONRELEASE) {
					self.value = value - increment
					false
				}
			else
				button.connect(SEL_COMMAND) {
					self.value = value + increment
					false
				}
			end
			button.create
		end
	end
end
