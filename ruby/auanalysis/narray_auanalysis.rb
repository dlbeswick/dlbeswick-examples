require 'narray'

class NArray
  def each_with_index(&block)
    i = 0
    
    each do |e|
      yield e, i
      i += 1
    end
  end
  
  def first
    self[0]
  end
  
  def inner_product
    temp = dup()
    temp.mul!(temp)
    temp.sum
  end
  
  def hilbert_norm
    Math.sqrt(inner_product())
  end
end

