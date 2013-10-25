# SourceView presents a view of a Source to the user.
require './Source/master'
require './Source/DevContainer'
require './Source/GUIValueModifier'
require './Source/sourcemodifier'
require 'std/view/ViewContainer'
require './Source/visualactuator'

module SourceViewContainers

	class Tabbed < ViewContainers::Tabbed
		def initialize(source, parent)
			super(source, parent, SourceView)
			
			# drag and drop
      if PlatformGUI.fox?
  			@fxWindow.dropEnable
  			@fxWindow.connect(SEL_DND_MOTION) {
  				if @fxWindow.offeredDNDType?(FROM_DRAGNDROP, FXWindow.urilistType)
  					@fxWindow.acceptDrop
  				end
  			}
  			@fxWindow.connect(SEL_DND_DROP) {
  				data = @fxWindow.getDNDData(FROM_DRAGNDROP, FXWindow.urilistType)
  				if !data.nil?
  					fname = /file:\/\/(.*)/.match(data)[1]
  					source.markUndo
  					SafeThread.new {
  						source.open(fname)
  					}
  				end
  			}
      else
        DNDArea.inject(self, $djgui.dragAndDrop)
        
		    # Master shouldn't really inherit from Source. That'll fix being able to drop tracks on it.
        dndarea_accept?(DNDData::Track) do
        	source.accepts_tracks?
        end
        
        dndarea_accept(DNDData::Track) do |data|
          source.markUndo
          source.open(data.track)
        end
      end
		end
	end

end

class SourceView < ViewBasicAttributeReference
  include Abstract
end

module SourceViews
  class OutputCommon < SourceView
    include PlatformGUISpecific
    
    def make_controls
      super
      
      @devicesBox = DataCombo.new(self, []) do |text, data|
        onDeviceSelected(data)
        target().save
      end
      
      layoutAddVertical(@devicesBox.gui)

      refreshDeviceList
    end
    
    def refreshDeviceList
      $djgui.app().audioLibrary.refreshDevices

      devices_info = []
      
      $djgui.app().audioLibrary.devices.each do |d|
        devices_info << d.description
      end
      
      @devicesBox.data = devices_info 

      @devicesBox.select_by_text(target().device.description)
    end
    
    def onDeviceSelected(device_name)
      puts "Sound device changing to #{device_name}."
      target().device_by_name = device_name
    end
  end
  
  class OutputWx < OutputCommon
    include Abstract
    include PlatformGUISpecific
    
    def initialize(source, parentWindow)
      super
    end
  end
  
	class OutputFox < OutputCommon
    include PlatformGUISpecific
    
		def initialize(source, parentWindow)
			super
			
			@topLayout = FXHorizontalFrame.new(parentWindow)
			@topLayout.create
			
			@devicesBox = FXComboBox.new(@topLayout, 1, nil, 0, COMBOBOX_STATIC)
			@devicesBox.numVisible = 8
			@devicesBox.layoutHints = LAYOUT_FILL_X
			@devicesBox.connect(SEL_COMMAND) {
				onDeviceSelected
				target().save
			}
			@devicesBox.create
			
			DevContainer.new(@topLayout) do |ctrl|
				ctrl.create
				
				FXButton.new(ctrl, 'pause') do |b|
					b.create
					b.connect(SEL_LEFTBUTTONPRESS) do
						source.device.stop
						0
					end
					b.connect(SEL_LEFTBUTTONRELEASE) do
						source.device.play
						0
					end
				end
			end
			
			ctrl = FXButton.new(@topLayout, 'refresh')
			ctrl.create
			ctrl.connect(SEL_COMMAND) {
				refreshDeviceList
			}
		end
	end
	
	# hack
	def self.addGain(source, parentWindow, midiIdx)
		a = VisualActuator::Slider.new(parentWindow)
		a.actuator_connect(source.modifierOfType(SourceModifiers::Gain))
		a
	end
	
	class Monitor < Output
	  suits MasterMonitor
	  
		def initialize(target, parentWindow)
			super
			
			source = target()
			
      if PlatformGUI.fox?
  			FXButton.new(@topLayout, 'sync') do |b|
  				b.create
  				b.connect(SEL_COMMAND) do
  					source.syncToOutput(SourceViews::Master.instances[0].target)
  				end
  			end
      else
        # disabled
  			#b = makeCheck(self, 'sync', source.sync) do |checked|
        #  source.sync = checked
        #end

        #layoutAddVertical(b)
      end
			
			a = VisualActuator::Slider.new(self)
			a.actuator_connect(target().modifierOfType(SourceModifiers::Crossfade))
			layoutFillVertical(a)
			
			a = VisualActuator::Slider.new(self)
			a.actuator_connect(target().modifierOfType(SourceModifiers::Gain))
			layoutFillVertical(a)
		end
		
		def description
			'Monitor'
		end
	end
	
	class Master < Output
		include EnumerableInstances #temporary -- for monitor sync
		
		suits MasterOutput
		
		def initialize(source, parentWindow)
			super
			
			# tbd: this line will cause a hang due to drb's exception handling. find out why.
      #get = Proc.new { target().recordPreference }
			
			get = Proc.new { target().record }
			set = Proc.new { |checked| target().record = checked; target().record = checked; target().save }
			@recording = GUIValueModifier::Check.new(self, 'Record?', get, set)
      layoutAddVertical(@recording)

			# master view
			a = VisualActuator::Slider.new(self)
			a.actuator_connect(target().modifierOfType(SourceModifiers::Crossfade))
			layoutFillVertical(a)
			
      target().modifiers.each do |m|
				if (m.viewable_kind_of?(SourceModifiers::Time))
					a = VisualActuator::Timebar.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
					
					a = VisualActuator::Shuttle.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				elsif m.viewable_kind_of?(SourceModifiers::Gain)
					layoutFillVertical(SourceViews.addGain(target(), self, 0))
				elsif (m.viewable_kind_of?(SourceModifiers::Pitch))
					a = VisualActuator::Slider.new(self, 90.0, 110.0)
					a.actuator_connect(m)
					layoutFillVertical(a)
					
					a = VisualActuator::PitchShuttle.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				elsif m.viewable_kind_of?(SourceModifiers::Filter)
					a = VisualActuator::Slider.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				end
      end
		end

		def description
			'Master'
		end
	end
	
	class SourceManipulator < SourceView
	  abstract
	  
		@@index = 0 # hack
		
		def initialize(source, parentWindow)
			super
			
			index = @@index
			@@index += 1

			target().modifiers.each do |m|
				if (m.viewable_kind_of?(SourceModifiers::Time))
					a = VisualActuator::Timebar.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
					
					a = VisualActuator::Shuttle.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				elsif m.viewable_kind_of?(SourceModifiers::Gain)
					layoutFillVertical(SourceViews.addGain(target(), self, 81 + 4*index))
				elsif (m.viewable_kind_of?(SourceModifiers::Pitch))
					a = VisualActuator::Slider.new(self, 90.0, 110.0)
					a.actuator_connect(m)
					layoutFillVertical(a)

					a = VisualActuator::PitchShuttle.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				elsif m.viewable_kind_of?(SourceModifiers::Filter)
					a = VisualActuator::Slider.new(self)
					a.actuator_connect(m)
					layoutFillVertical(a)
				end
			end
		end
	end

  class StreamManipulatorBase < SourceManipulator
    abstract
    
    suits SourceStream

    def initialize(source, parentWindow)
      super
      
      @undo_status = Source::NotifyUndoStatus.new(self) { |status| updateUndoStatus(status) }
      target().notify_listen(@undo_status)
      updateUndoStatus(target().hasUndo?)
    end

    def description
      'Manipulator'
    end

    def updateUndoStatus(status)
      PlatformGUI.specific
    end
  end

  class StreamManipulatorWx < StreamManipulatorBase
    include PlatformGUISpecific

    def updateUndoStatus(status)
      #tbd
    end
  end
  
	class StreamManipulatorFox < StreamManipulatorBase
    include PlatformGUISpecific
    
		def initialize(source, parentWindow)
			super
			
			@undo = FXButton.new(parentWindow, "Undo")
			@undo.create
			@undo.connect(SEL_COMMAND) { SafeThread.new { target().undo } }
		end
    
    def updateUndoStatus(status)
      @undo.enabled = status
    end
	end
end
