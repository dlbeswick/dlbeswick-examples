# Methods extending the built-in types.

class Array
  def collect_with_index(&block)
    i = 0
    collect() do |e|
      result = yield e, i
      i += 1 
      result
    end
  end
end