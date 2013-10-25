require './Source/source'
require './Source/sourcemodifier'
require './Source/analysis/song_transition_event'
require './Source/analysis/song_play_event'

class SourceStreamDSound < SourceStream
  attr_reader :mp3
  attr_reader :fname
  attr_reader :current_track
  attr_reader :song_play_event
  
  def initialize(app, name)
    super(name, app.audioLibrary)

    self.undoSet.persistent(:fname)

    @open_dont_set_default_modifiers = []
      
    @newPosition = -1
    @fname = Path.new
    @pausePos = 0

    # tbd: this will fail if the sound library decides to use a different latency to that of 
    # audioLibrary.latency 
    @mp3 = Dsound::SoundBufferMedia.new(app.audioLibrary.native_library_instance, app.audioLibrary.latency)
    @mp3.setEnabled(false)

    @nodeClean = Dsound::ModifierNode.new
    @nodeClean.addInput(@mp3.requestOutput)

    @sine = Dsound::Sine.new
    @sine.setEnabled(false)
    @nodeClean.addModifier(@sine)

    @gaineffect_volume_norm = Dsound::GainEffect.new
    @nodeClean.addModifier(@gaineffect_volume_norm, 80)
    
    @gainEffect = Dsound::GainEffect.new
    @nodeClean.addModifier(@gainEffect, 90)

    @frequency = Dsound::Frequency.new
    @nodeClean.addModifier(@frequency, 100)

    @nodeEffect = Dsound::ModifierNode.new
    @nodeEffect.addInput(@nodeClean.requestOutput)
  end
  
  def close
    stop
    @opened = false
    super
  end
  
  # the output of the "clean" node gets sent to the monitor, and the output of "modifiable" gets sent to the master.
  # the output of "modifiable" includes the output from "clean".
  # if an effect is added to "modifiable", then the effect is not heard on the monitor channel.
  # tbd: find a more extensible way to do this -- node connection editor?
  def outputClean
    @nodeClean
  end

  def output
    @nodeEffect
  end

  def open?
    @opened == true
  end

  def open(track)
    notifyNameChange

    close
    
    raise "No output connected." if !output
    
    if @current_track
      $djApp.on_song_transition
    end
    
    @current_track = track
    @fname = track.path
    log { "Opening audio file #{@fname.to_sys}..." }        
    $stdout.flush
    
    norm_gain = if track.volume_normalization_db?
      track.volume_normalization_db.db - 3.0
    else
      log { "#{@fname}: No volume normalization data. Applying default gain." }
      -3.0
    end

    has_default_gain = norm_gain == 0.0
   
    @gaineffect_volume_norm.setGain(norm_gain)
    
    log { "#{@fname}: Volume normalization is #{@gaineffect_volume_norm.gain} db." }
    
    @mp3.open(@fname.to_sys)
    
    # tbd: find out why I did this
    while opening()
      sleep(0.1)
    end

    @opened = true

    log { "#{@fname}: Sample rate is #{sample_rate()}" }

    SourceModifierBatchUpdate.new(modifiers(), self) do |modifier|
      modifier.set_defaults if modifier_should_set_defaults_on_open?(modifier)
    end
  
    # if no volume normalization value is present, then set the sourcemodifier's value to the
    # default gain value as visual feedback to the user.
    if has_default_gain
      gain_modifier = modifierOfType(SourceModifiers::Gain)
      gain_modifier.value = $djApp.volume_normalization_default.db
    end
    
    notifyNameChange
    
    if $use_event_db
      @song_play_event = $djApp.event_database_session.new_event do 
        SongPlayEvent.new(@current_track) 
      end
    end
      
    nil
  end
  
  def finalize_song_play_event
    if $use_event_db
      @song_play_event.pitch = modifierOfType(SourceModifiers::Pitch).value
    end
  end
  
  def modifier_should_set_defaults_on_open(modifier, should_set)
    raise "Modifier is not owned by this source" if !modifiers().include?(modifier)
    
    if !should_set
      if !@open_dont_set_default_modifiers.include?(modifier)
        @open_dont_set_default_modifiers << modifier  
      end
    else
      @open_dont_set_default_modifiers.delete(modifier)
    end
  end
  
  def modifier_should_set_defaults_on_open?(modifier)
    !@open_dont_set_default_modifiers.include?(modifier)
  end
  
  def play
    return if !@mp3 || !@mp3.opened
    @mp3.setEnabled(true)
    super
  end
  
  def playing?
    @mp3.enabled
  end
  
  def pause
    return if !@mp3 || !@mp3.opened
    @mp3.setEnabled(false)
    super
  end
  
  def stop
    return if !@mp3 || !@mp3.opened
    @mp3.setEnabled(false)
    super
  end

  def length
    return 1 if !@mp3
    return @mp3.lengthMS
  end
  
  def position
    return 0 if !@mp3
    return @newPosition if @newPosition != -1
    return @mp3.progress
  end
  
  def position=(pos_ms)
    waitForLengthCalculation()
    @newPosition = pos_ms
    super
  end

  def gain=(db)
    super
    @gainEffect.setGain(db)
  end

  def opening
    return @mp3.opening
  end

  def description
    return "Loading..." if opening == true
    return @fname.file.to_s
  end
  
  def commit
    super
    
    return if !@mp3

    if @newPosition != -1
      if @newPosition >= @mp3.lengthMS
        @mp3.setProgress(@mp3.lengthMS)
        stop
      else
        @mp3.setProgress(@newPosition)
      end
      
      @newPosition = -1
    end

    #pitch = modifierOfType(SourceModifiers::Pitch).value
    #@sine.frequency = 88#Mapping.linear(pitch, 0.9, 1.1, 80.0, 1000.0)
    @gainEffect.setGain(gain())
  end
  
  def modifiable
    @nodeEffect
  end

  def sample_rate
    @mp3.rate
  end

  def undo
    super
    open(@fname.to_sys)
    @mp3.waitForLengthCalculation
  end

  def visualisable
    @nodeEffect.visualisable
  end
  
:protected
  def waitForLengthCalculation
    if @mp3
      @mp3.waitForLengthCalculation
    end
  end
end
