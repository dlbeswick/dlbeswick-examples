require './Source/analysis/resource_event'

class SongPlayEvent < ResourceEvent
  attr_accessor :pitch

  def initialize(resource)
    super
  end
end
