require './Source/analysis/application_session_owned_event'
require './Source/analysis/song_play_event'

class SongTransitionEvent < ApplicationSessionOwnedEvent
  persistent_accessor :preceding_event
  persistent_accessor :song_play_events
  
  def initialize(preceding_event, song_play_events)
    super()
    @preceding_event = preceding_event
    @song_play_events = song_play_events.dup
  end
end
