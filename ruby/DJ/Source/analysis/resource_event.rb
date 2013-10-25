require './Source/analysis/application_session_owned_event'

class ResourceEvent < ApplicationSessionOwnedEvent
  attr_accessor :resource
  
  def initialize(resource)
    super()
    @resource = resource
  end
end
