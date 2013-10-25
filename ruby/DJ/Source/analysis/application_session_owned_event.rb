require './Source/analysis/application_event'
require 'std/unique'

class ApplicationSessionOwnedEvent < ApplicationEvent
  include Unique
  
  use_id :auto_id
  use_id_context :application_session, :application_events
  
  persistent_reader :application_session
  persistent_reader :auto_id
  
  def _application_session=(application_session)
    @application_session = application_session
  end
  
  def auto_id
    @auto_id ||= @application_session.make_event_auto_id
  end
end
