require 'std/ConfigYAML'
require 'std/unique'
require 'std/view/Viewable'

class Artist
  include ConfigYAML
  include Comparable
  include Unique
  include Viewable
  
  persistent_reader :name
  persistent_reader :library
  
  use_id :name
  use_id_context :library, :artists
  
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
  
  def various?
    equal?(Artist.various(library()))
  end
  
  # tbd: fix 'unique' to allow a set of objects to be unique across a global context. The 'library' parameter
  # will then not be required.
  def self.various(library)
    raise "Cannot retrieve Various Artist entry from nil library." if !library
    library.artists.add_or_retrieve("Various Artists")
  end
end
