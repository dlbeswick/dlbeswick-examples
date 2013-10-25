# Source objects are the originators of sound, for example an mp3 file. Sources feed into the
# Master object.

require 'std/UniqueIDConfig'
require 'std/view/Viewable'
require 'drb'
require 'mutex_m'

class Source
  include UniqueIDConfig
  include Viewable
  include DRb::DRbUndumped

  class NotifyUndoStatus < Notify::Base
    def onChange(status)
      @callback.call(status)
    end
  end

  class NotifyPositionChange < Notify::Base
    def onChange
      @callback.call
    end
  end

  class NotifyFrequencyChange < Notify::Base
    def onChange
      @callback.call
    end
  end

  class NotifyPlay < Notify::Callback
  end

  attr_reader :output
  attr_reader :name
  attr_reader :playing

  attr_accessor :gain
  
  UpdateFreq = 0.25

  def initialize(name, audioLibrary)
    ensure_id
    
    @undo = ConfigSet.new(self)
    @undo.persistent(:playing)
    @undo.persistent_accessor(:position)
    @undo.persistent(:gain)
    @undo.persistent(:modifiers)
    
    @modifiers = []
    @modifiers.extend(Mutex_m)  
    @name = name
    @gain = 0.0
    @playSegment = nil
    audioLibrary.addSource(self)
    
    SafeThread.new { 
      while !SafeThread.exit?
        sleep(UpdateFreq)
        update 
      end
    }
  end

  # Master shouldn't really inherit from Source. That'll fix being able to drop tracks on it.
  def accepts_tracks?
    true
  end
  
  def close
    modifiers().each do |m|
      m.onSourceClose
    end
  end
	
  def addModifier(modifier)
    @modifiers.synchronize do
      @modifiers << modifier
    end
    modifier
  end
	
  def modifiers
    @modifiers.synchronize do
      @modifiers.dup
    end
  end

  def modifierOfType(type)
    return nil if !@modifiers # can be called by syncToOutput
    
    modifiers().each do |m|
      return m if m.kind_of?(type)
    end
    
    nil
  end
  
  def onLibraryInit(audioLibrary)
  end
	
  def on_master_window_set(audio_library)
  end
	
  def play
    notify_each(NotifyPlay) { |n| n.notify } if !@playing
    @playing = true
  end
	
  def playSegment(range)
    stop # delete this?
    @playSegment = range
    self.position = @playSegment.begin
    commit
    play
  end
	
  def pause
    @playing = false
  end
	
  def stop
    @playing = false
    @stopAtPosition = -1
  end
  
  def modify
    @gain = 0.0
    modifiers().each do |m|
      m.modify
    end
  end
	
  def commit
  end
	
  # only two return values are valid for this function
  # nil means that the source is of infinite length
  # otherwise, this function should return the length of the stream in ms
  def length
    return nil
  end

  # returns normalized stream position. valid return values are 0 to 1.
  def position
    return 0
  end
  
  def position=(pos)
    notify_each(NotifyPositionChange) { |n| n.onChange }
  end
	
  # return a detailed description of the stream (i.e. filename)
  def description
  end
	
  def hasUndo?
    @hasUndo
  end

  def markUndo
    @undo.save
    @hasUndo = true
    notifyUndoStatus
  end
	
  def undo
    @undo.load
    @hasUndo = false
    notifyUndoStatus
  end
  
  def modifiable
    raise 'Source cannot be modified.'
  end
  
  def viewable_name
    name()
  end
  
  def visualisable
    raise 'Source cannot be visualised.'
  end
	
:protected
  def undoSet
    @undo
  end
	
  def update
    if shouldStop?
      stop
      self.position = @playSegment.begin
      @playSegment = nil
      commit
    end
  end
  
  def shouldStop?
    return @playSegment != nil && self.position >= @playSegment.end
  end
  
  def notifyUndoStatus
    notify_each(NotifyUndoStatus) { |n| n.onChange(hasUndo?) }
  end
  
  def notifyNameChange
    notify_each(NotifyNameChange) { |n| n.onChange }
  end
end

class SourceStream < Source
end
