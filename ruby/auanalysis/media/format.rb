require 'std/ConfigYAML'
require 'std/unique'
require 'std/UniqueIDConfig'
require 'std/view/Viewable'

class Format
  include Comparable
  include ConfigYAML
  include Unique
  include Viewable
  
  persistent_reader :name
  persistent_reader :library

  use_id :name
  use_id_context :library, :formats
  
  def initialize(name, library)
    @name = name
    @library = library
    bind_id(@name) # tbd: find a way to avoid this
  end
  
  def <=>(rhs)
    @name <=> rhs.name
  end
  
  def to_s
    name().dup
  end
end
