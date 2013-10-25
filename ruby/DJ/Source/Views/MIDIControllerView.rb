require './Source/Views/ActuatorHostView'
require './Source/DataSelector'
require './Source/midiactuator'
require './Source/MIDIController'
require './Source/Views/MIDIActuatorView'

class MIDIControllerView < ActuatorHostView
  suits MIDIController

  def description
    if !target().device
      'MIDI Controller (no device)'
    else
      target().device.name
    end
  end

  def make_controls
    super
    
    @devicesBox = DataCombo.new(self, []) do |text|
      onDeviceSelected(text)
    end
    layoutAddVertical(@devicesBox.gui)
    
    refreshDeviceList
  end
  
  def validActuatorClass?(c)
    c.viewable_kind_of?(MIDIActuator)
  end
  
protected
  def additionClasses
    MIDIActuator::Base.subclasses
  end
  
  def onDeviceSelected(device_name)
    log { "MIDI device changing to #{device_name}." }
    target.device_by_name = device_name
    descriptionChanged()
  end

  def refreshDeviceList
    note { "Refresh MIDI devices." }
    @devicesBox.clear
    
#   refresh midi devices here $djApp.audioLibrary.refreshDevices
    data = []
      
    $djgui.app().midiDevicesIn.each do |d|
      data << d.name
    end
    
    @devicesBox.data = data
    
    #if target().device
    #  @devicesBox.select(target().device)
    #end
    note { "Refresh MIDI devices done." }
  end
end

MIDIControllerView.log = true
