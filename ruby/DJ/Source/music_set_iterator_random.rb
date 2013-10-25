require './Source/music_set_iterator'

class MusicSetIteratorRandom < MusicSetIterator
  def initialize(collection)
    @collection = collection
    @index = -1
  end

  def start
    @index = -1
		@tracks = @collection.all_tracks!.sort_by { rand() }
  end

  def prev
    @index = [-1, @index - 1].max
  end

  def next
    @index = [@index + 1, @tracks.length].min
  end

  def begin?
    @index == -1
  end

  def end?
    @index == @collection.tracks_length
  end

  def current
    if begin? || end?
      nil
    else
      @tracks[@index]
    end
  end
end
