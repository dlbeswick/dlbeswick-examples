require 'transform/export/transform_task'

class TaskSourceAlbum < TransformTask
  def initialize(source_album)
    super()
    @source_album = source_album
  end
  
  def describe
    "#{super} (#{@source_album.to_s})"
  end
end
