require './Source/music_set_iterator'

class MusicSetIteratorSequential < MusicSetIterator
  def initialize(collection)
    @collection = collection
    @index = -1
  end

  def start
    @index = -1
    debugger
    @count = @collection.tracks_length
  end

  def prev
    @index = [-1, @index - 1].max
  end

  def next
    @index = [@index + 1, @count].min
  end

  def begin?
    @index.nil?
  end

  def end?
    @index == @count
  end

  def current
    if begin? || end?
      nil
    else
      @collection.track_at_idx(@index)
    end
  end
end
