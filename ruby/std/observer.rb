module Observer
  def observe(who, symbol, my_callback_symbol)
    raise "Nil observer." if who == nil
    who.observableattributes_add_observer(symbol, self, my_callback_symbol)
  end
  
  def observer_callback_executing(who_observed, symbol)
    @observer_current_callback_who ||= []
    @observer_current_callback_symbol ||= []
    @observer_current_callback_who.push(who_observed)
    @observer_current_callback_symbol.push(symbol)
  end

  def observer_callback_finished
    @observer_current_callback_who.pop
    @observer_current_callback_symbol.pop
  end
  
  # stop subscribing to value changes.
  # if called with no parameters when inside an observer callback, the observer that triggered the callback is unsubscribed.
  def unobserve(who=nil, symbol=nil)
    if who == nil
      raise "unobserve was called without parameters when outside of an observer callback function." if !@observer_current_callback_who || @observer_current_callback_who.empty?
      who = @observer_current_callback_who.last
      symbol = @observer_current_callback_symbol.last
    end
    
    raise "Nil observer." if who == nil
    raise "Nil symbol." if symbol == nil
    who.observableattributes_remove_observer(symbol, self)
  end
end