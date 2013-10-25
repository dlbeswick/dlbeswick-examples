require './Source/music_set_iterator'

class MusicSetIteratorConcat < MusicSetIterator
  def initialize(iterators)
    @iterators = iterators
    @finished_iterators = []
    @current_iterators = []
  end

  def start
    @finished_iterators = []
    @current_iterators = @iterators.dup
    current_iterator().start if not current_iterator().nil?
  end

  def current_iterator
    @current_iterators.first
  end

  def begin?
    @finished_iterators.empty?
  end

  def end?
    @current_iterators.empty?
  end

  def prev
    return if current_iterator().nil?

    if current_iterator().begin?
      if !@finished_iterators.empty?
        @current_iterators = @finished_iterators.first
        @finished_iterators = @finished_iterators[1..-1]
        current_iterator().start
      end
    else
      current_iterator().prev
    end
  end

  def next
    return if current_iterator().nil?

    if current_iterator().end?
      if !@current_iterators.empty?
        @finished_iterators = @current_iterators.first
        @current_iterators = @current_iterators[1..-1]
        current_iterator().start
      end
    else
      current_iterator().next
    end
  end

  def current
    if not current_iterator().nil?
      current_iterator().current
    else
      nil
    end
  end
end
