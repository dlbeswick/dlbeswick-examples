module ObservableAttributes
  module ClientAttributes
    def attr_observable(symbol)
      self.module_eval <<-EOF
        def #{symbol}=(value)
          old_value = @#{symbol}
          @#{symbol} = value
          observableattributes_changed(#{symbol}, old_value, @#{symbol})
        end
      EOF
    end
  end

  def self.append_features(client)
    super
    client.extend(ClientAttributes)
  end

  def observableattributes_changed(symbol, old_value, value)
    @observableattributes_observers ||= Hash.new([])
    @observableattributes_observers[symbol].dup.each do |observer_record|
      observer = observer_record[0]
      observer_method = observer_record[1]
      
      observer.observer_callback_executing(self, symbol)
      
      if observer_method.arity == 0
        observer_method.call()
      elsif observer_method.arity == 1
        observer_method.call(value)
      elsif observer_method.arity == 2
        observer_method.call(self, value)
      else
        observer_method.call(self, old_value, value)
      end

      observer.observer_callback_finished
    end
  end
  
  def observableattributes_add_observer(symbol, observer, observer_symbol_callback)
    @observableattributes_observers ||= Hash.new([])
    @observableattributes_observers[symbol] << [observer, observer.method(observer_symbol_callback)]
  end

  def observableattributes_remove_observer(symbol, observer)
    @observableattributes_observers ||= Hash.new([])
    @observableattributes_observers[symbol].delete_if { |x| x[0] == observer.object_id }
  end
end
