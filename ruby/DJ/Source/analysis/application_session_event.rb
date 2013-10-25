require './Source/analysis/application_event'
require './Source/analysis/application_session_owned_event'
require 'std/unique'

# A single run of an application
class ApplicationSessionEvent < ApplicationEvent
  include Unique
  include UniqueHost
  
  unique_id_registry ApplicationSessionOwnedEvent, :application_events
  use_id_context :event_database, :application_sessions
  
  use_id :auto_id
  persistent_reader :auto_id
  persistent_reader :event_database
  persistent :event_auto_id
  
  def initialize
    super
    @event_auto_id = 0
  end
  
  def auto_id
    @auto_id ||= @event_database.make_session_auto_id
  end
  
  def make_event_auto_id
    result = @event_auto_id
    @event_auto_id += 1
    result
  end  

  def _event_database=(event_database)
    @event_database = event_database
  end
    
  def new_event(&block)
    event = block.yield
    event._application_session = self
    application_events().add(event)
    event
  end
end
