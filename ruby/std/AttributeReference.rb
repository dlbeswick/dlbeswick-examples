require 'std/EnumerableInstances'

module AttributeReference
  class Base
    include Comparable
    
    attr_accessor :host
    attr_accessor :ownerReference
    
    def initialize(host=nil, ownerReference=nil)
      @host = host
      @ownerReference = ownerReference
    end
    
    def <=>(rhs)
      if rhs.kind_of?(Base)
        if attributereference_compares_to?(rhs)
          attributereference_compare(rhs)
        else
          nil
        end
      else
        nil
      end
    end
    
    def eql?(rhs)
      (self <=> rhs) == 0
    end
    
    def attributereference_compares_to?(rhs)
      false
    end

    def hash
      nil
    end
    
    def get(host=@host)
    end
    
    def get_target_class
      get().viewable_class
      # WILL NOT WORK IN THE LONG RUN! TEST ONLY
      #@target_class ||= get().viewable_class
    end
    
    def set(incomingVar, host=@host)
    end

    def to_s_detail()
      if host()
        "host #{host().class}"
      else
        nil
      end
    end
    
    def to_s
      result = "#{self.class.name}"
      detail = to_s_detail
      if detail
        result += " (#{detail})"
      end
      result
    end
    
    # derived class should return true here if a 'set' call is meaningful.
    def writable?
      false
    end
  end

  # a simple reference to a standalone object.
  # tbd: this doesn't seem appropriate to be subclassed from AttributeReference.
  # it may be more appropriate as a parent class. 
  # Used in the Range view.
  class Object < Base
    def attributereference_compares_to?(rhs)
      rhs.kind_of?(Object)
    end
    
    def attributereference_compare(rhs)
      host().object_id <=> rhs.host.object_id
    end
    
    def hash
      host().object_id
    end
    
    def get(host=nil)
      raise "The only appropriate host for this object is given on creation." if host
      @host
    end

    def set(incomingVar, host=nil)
      @host = incomingVar
    end

    def writable?
      true
    end
  end
  
  class ObjectReadOnly < Object
    def get(host=nil)
      raise "The only appropriate host for this object is given on creation." if host
      @host
    end

    def set(incomingVar, host=nil)
      raise "This attribute is read-only."
    end

    def writable?
      false
    end
  end
  
  class AttributeSymbol < Base
    attr_reader :symbol

    def initialize(symbol, host=nil, ownerReference=nil)
      super(host, ownerReference)
      self.symbol = symbol
    end

    def attributereference_compares_to?(rhs)
      rhs.kind_of?(AttributeSymbol)
    end
    
    def attributereference_compare(rhs)
      symbol().to_s <=> rhs.symbol.to_s 
    end

    def hash
      symbol().hash
    end
    
    def symbol=(symbol)
      raise "Symbol expected, got '#{symbol.class}'" if !symbol.kind_of?(Symbol) && !symbol.kind_of?(NilClass)
      @symbol = symbol
    end

    def to_s_detail()
      super_detail = super
      
      if super_detail
        "#{super_detail}.#{symbol()}"
      else
        "<host>.#{symbol()}"
      end
    end
  end
  
  # A reference to an instance variable.
  class Instance < AttributeSymbol
    def get(host=@host)
      return nil if !host
      host.instance_variable_get('@'+@symbol.to_s)
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      host.instance_variable_set("@#{@symbol}", incomingVar)
    end

    def writable?
      true
    end
  end
  
  # Describes data accessible via attr_reader.
  class Reader < AttributeSymbol
    def self.suitsSymbol?(host, symbol)
      host.respond_to?(symbol)
    end

    def get(host=@host)
      raise "No host supplied to either 'get' or 'initialize'." if !host
      
      # xdrb
      if host == @host
        @verified_symbol ||= begin
                               if host.respond_to?(:viewable_class)
                                 raise "Host of class '#{host.viewable_class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
                               else
                                 raise "Host of class '#{host.class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
                               end
                               
                               true
                             end
      else
        if host.respond_to?(:viewable_class)
          raise "Host of class '#{host.viewable_class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
        else
          raise "Host of class '#{host.class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
        end
        
        true
      end

      host.send(symbol())
    end
    
    def set(incomingVar, host=@host)
      raise "Attribute reader '#{@symbol}' for host of class '#{host.class}' cannot be 'set'."
    end

    def writable?
      false
    end
  end
  
  # tbd: clean up duplicate code in Accessor
  # Describes data accessible via attr_writer.
  class Writer < AttributeSymbol
    def self.suitsSymbol?(host, symbol)
      host.respond_to?("#{symbol}=")
    end

    def get(host=@host)
      raise "Attribute writer '#{@symbol}' for host of class '#{host.class}' cannot be read with 'get'."
    end
    
    def set(incomingVar, host=@host)
      raise "No host supplied to either 'set' or 'initialize'." if host.nil?
      raise "Host of class '#{host.class}' has no writer called '#{symbol}'." if !Writer.suitsSymbol?(host, symbol)
      host.send("#{@symbol}=", incomingVar)
    end
    
    def writable?
      true
    end
  end

  # Describes data accessible via attr_reader. Allows set via instance_variable_set.
  class ReaderInstanceSet < Reader
    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      host.instance_variable_set("@#{symbol}", incomingVar)
    end
    
    def writable?
      true
    end
  end
  
  # Describes data accessible via attr_writer. Allows retrieval via instance_variable_get.
  class WriterInstanceGet < Writer
    def get(host=@host)
      return nil if !host
      host.instance_variable_get('@'+@symbol.to_s)
    end
  end
  
  # Describes data accessible via attr_accessor.
  class Accessor < Reader
    def self.suitsSymbol?(host, symbol)
      Reader.suitsSymbol?(host, symbol) && hostHasWriter?(host, symbol)
    end
    
    def self.hostHasWriter?(host, symbol)
      host.respond_to?(symbol.to_s + "=")
    end
    
    def set(incomingVar, host=@host)
      raise "No host supplied to either 'set' or 'initialize'." if host.nil?
      raise "Host of class '#{host.class}' has no writer called '#{symbol}'." if !Accessor.hostHasWriter?(host, symbol)

      host.send("#{@symbol}=", incomingVar)
    end
    
    def writable?
      true
    end
  end

  class ObjectReference < AttributeSymbol
    def get(host=@host)
      return nil if !host
      instanceVar = host.instance_variable_get('@'+@symbol)

      if instanceVar
        if !instanceVar.respond_to?(:unique_id)
          raise "'#{instanceVar}': persistent objects saved by reference must include UniqueID module."
        end
        
        instanceVar.unique_id
      else
        nil
      end
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?

      if incomingVar != nil
        begin
          host.instance_eval("@#{@symbol} = self.class.object_for_unique_id(incomingVar)")
        rescue Exception=>e
          puts "Error restoring config variable reference: #{e}"
        end
      end
      super
    end
  end

  class ArrayElement < Base
    attr_accessor :idx
    
    def initialize(idx, host=nil, ownerReference=nil)
      super(host, ownerReference)
      ensure_host_is_array()
      @idx = idx
    end

    def attributereference_compares_to?(rhs)
      rhs.kind_of?(ArrayElement)
    end
    
    def attributereference_compare(rhs)
      idx() <=> rhs.idx 
    end
    
    def hash
      idx().hash
    end
    
    def attributeIsNestedArray?
      # tbd: both types of array elements should share a base class
      self.ownerReference && (self.ownerReference.kind_of?(ArrayElementSymbol) || self.ownerReference.kind_of?(ArrayElement))
    end
    
    def get(host=@host)
      return host[@idx]
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      host[@idx] = incomingVar
      super
    end
    
    def writable?
      true
    end
    
    protected
    def ensure_host_is_array
      raise "ArrayElements must be hosted by Arrays" if !host().kind_of?(Array)
    end
  end

  # doesn't store a reference to an array, but instead to its host and the symbol used to access it.
  # should generally be used unless it's not possible, i.e. unless it's unavoidable that the reference
  # be to an array instanced outside of an object host. 
  class ArrayElementSymbol < Reader
    attr_accessor :idx
    
    def initialize(symbol, idx, host=nil, ownerReference=nil)
      super(symbol, host, ownerReference)
      @idx = idx
    end

    def attributereference_compares_to?(rhs)
      rhs.kind_of?(ArrayElementSymbol)
    end
    
    def attributereference_compare(rhs)
      idx() <=> rhs.idx 
    end
    
    def hash
      idx().hash
    end
    
    def attributeIsNestedArray?
      # tbd: both types of array elements should share a base class
      self.ownerReference && (self.ownerReference.kind_of?(ArrayElementSymbol) || self.ownerReference.kind_of?(ArrayElement)) 
    end
    
    def get(host=@host)
      super(host)[@idx]
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      get(host)[@idx] = incomingVar
    end
    
    def writable?
      true
    end
  end
  
  class HashKeyElement < AttributeSymbol
    attr_accessor :key
    
    # 'symbol' is an instance variable name without the '@'.
    def initialize(key, symbol, host=nil, ownerReference=nil)
      super(symbol, host, ownerReference)
      @key = key
    end

    def attributereference_compares_to?(rhs)
      rhs.kind_of?(HashKeyElement)
    end
    
    def attributereference_compare(rhs)
      key() <=> rhs.key 
    end
    
    def hash_host(host)
      host.instance_variable_set('@'+symbol().to_s, {}) if !host.instance_variable_defined?('@'+symbol().to_s)
      host.instance_variable_get('@'+symbol().to_s)
    end

    def get(host=@host)
      return @key
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      value = self.hash_host(host)[@key]
      self.hash_host(host).delete(@key)
      @key = incomingVar
      self.hash_host(host)[@key] = value
      ownerReference.set(self.hash_host(host)) if ownerReference
      super
    end
    
    def writable?
      true
    end
  end

  class HashValueElement < AttributeSymbol
    def initialize(key_attribute, host=nil, ownerReference=nil)
      super(nil, host, ownerReference)
      @key_attribute = key_attribute
    end

    def attributereference_compares_to?(rhs)
      rhs.kind_of?(HashValueElement)
    end
    
    def attributereference_compare(rhs)
      key() <=> rhs.key
    end
    
    def hash_host(host)
      @key_attribute.hash_host(host)
    end
    
    def key
      @key_attribute.key()
    end

    def get(host=@host)
      hash_host(host)[key()]
    end

    def set(incomingVar, host=@host)
      raise "Nil host." if host.nil?
      hash_host(host)[key()] = incomingVar 
      ownerReference.set(hash_host(host)) if ownerReference
      super
    end

    def symbol
      @key_attribute.symbol()
    end
    
    def writable?
      true
    end

    def to_s
      "#{super} key: #{key()}"
    end
  end
  
  class RangeFirst < Base
    def get(host=@host)
      host.first
    end
    
    def set(value, host=@host)
      raise "Nil host." if host.nil?
      if host.exclude_end?
        ownerReference().set(value...host.last)
      else
        ownerReference().set(value..host.last)
      end
    end
    
    def writable?
      true
    end
  end

  class RangeLast < Base
    def get(host=@host)
      host.last
    end
    
    def set(value, host=@host)
      raise "Nil host." if host.nil?
      if host.exclude_end?
        ownerReference().set(host.first...value)
      else
        ownerReference().set(host.first..value)
      end
    end
    
    def writable?
      true
    end
  end
end
