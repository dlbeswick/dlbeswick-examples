require 'std/abstract'

module ModificationWatcher
  include Abstract
  
  def modificationwatcher_clean
    abstract()
  end
  
  def modificationwatcher_dirty
    abstract()
  end
  
  def modificationwatcher_dirty?
    abstract()
  end
end

module FileModificationWatcher
  attr_reader :time_last_cleaned
  
  def modificationwatcher_clean
    @time_last_cleaned = Time.now
  end
  
  def modificationwatcher_dirty
    @time_last_cleaned = Time.at(0)
  end
  
  def modificationwatcher_dirty?
    return true if !time_last_cleaned()
    raise "watch_path has not been called." if !@filemodificationwatcher_block
    path = @filemodificationwatcher_block.call
    return true if !path
    mod_time = @filemodificationwatcher_block.call.stat.mtime
    @time_last_cleaned < mod_time
  end
  
  def watch_path(&block)
    @filemodificationwatcher_block = block 
  end
end
