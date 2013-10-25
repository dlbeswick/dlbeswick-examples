require './Source/analysis/application_event'
require './Source/analysis/application_session_event'
require './Source/analysis/relational_db'

class EventDatabase
  include UniqueHost
  include Unique

  use_id :singleton_id_context
  use_id_context :singleton_id_context, :singleton_id_context
  
  unique_id_registry ApplicationEvent, :application_sessions

  persistent_reader :session_auto_id
  
  def initialize
    @session_auto_id = 0
    @db = RelationalDB.new(configPath().with_extension("sqlite"))
  end
  
  def close
    @db.close
  end

  def configElementName
    self.class.name
  end
  
  def _hash
    @singleton_registry_hash ||= {}
  end
  
  def singleton_id_context
    self
  end
  
  def make_session_auto_id
    result = @session_auto_id
    @session_auto_id += 1
    result
  end  
  
  def new_session
    event = ApplicationSessionEvent.new
    event._event_database = self
    application_sessions().add(event)
    event
  end
  
  def save_session(session)
    @db.store(self)
  end
end
