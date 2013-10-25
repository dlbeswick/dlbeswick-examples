# extensions to Range object to aid use as encapsulation of an interval
class Range
  def &(other)
    this = Range.normalise_inclusive(self)
    other = Range.normalise_inclusive(other)
    
    min_last = [this.last, other.last].min
    max_first = [this.first, other.first].max

    return nil if max_first > min_last
    
    (max_first..min_last).normalise(self)
  end
  
  def self.normalise_exclusive(what)
    if what.exclude_end?
      what
    else
      (what.first...what.last + 1)
    end
  end

  def self.normalise_inclusive(what)
    if !what.exclude_end?
      what
    else
      (what.first..what.last - 1)
    end
  end
  
  def it_end
    if exclude_end?
      last()
    else
      lsat() - 1
    end
  end
  
  def normalise(inclusive=true)
    if inclusive
      Range.normalise_inclusive(self)
    else
      Range.normalise_exclusive(self)
    end
  end
  
  def normalise_inclusive
    Range.normalise_inclusive(self)
  end
  
  def normalise_exclusive
    Range.normalise_exclusive(self)
  end

  def normalise(reference)
    if reference.exclude_end? == exclude_end?
      self
    else
      if reference.exclude_end?
        (first..last-1)
      else
        (first...last+1)
      end    
    end
  end
        
  def length
    if exclude_end?
      last() - first()
    else
      last() - first() + 1
    end
  end
  
  def include_range?(other)
    this = Range.normalise_inclusive(self)
    other = Range.normalise_inclusive(other)
    other.first >= this.first && other.last <= this.last
  end
  
  def last_value
    if exclude_end?
      last() - 1
    else
      last()
    end
  end
end
