require 'abstract'

class ValidatingArray < Array
  include Abstract
  
  def fill(*args)
    _validate(*args)
    super
  end
  
  def push(*args)
    _validate(*args)
    super
  end
  
  def replace(*args)
    _validate(*args)
    super
  end
  
  def []=(*args)
    _validate(*args)
    super
  end
  
  def <<(*args)
    _validate(*args)
    super 
  end
  
  def +(*args)
    _validate(*args)
    super 
  end
  
  def -(*args)
    _validate(*args)
    super 
  end
  
  def |(*args)
    _validate(*args)
    super 
  end

  def &(*args)
    _validate(*args)
    super
  end

protected
  def validate(element)
    abstract
  end

private
  def _validate(*obj)
    case obj[0]
    when Array
      obj.each do |e|
        validate(e)
      end
    else
      validate(obj)
    end
  end
end
