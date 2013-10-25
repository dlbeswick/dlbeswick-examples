require 'transform/export/transform_task'

class TaskSourceTrack < TransformTask
  def initialize(source_track)
    super()
    @source_track = source_track
  end
  
  def describe
    "#{super} (#{@source_track.track.to_s})"
  end
end
