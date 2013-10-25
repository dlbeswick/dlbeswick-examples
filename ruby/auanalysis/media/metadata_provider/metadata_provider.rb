require 'std/abstract'

class MetadataProvider
  include Abstract
  
  # TBD: Make this transient. Enforce that it cannot be saved.
  attr_reader :library
  
  def initialize
    @library = Library.new_transient
  end
  
  def with_catalogue_id(cat_id)
    abstract
  end

  def with_artist(artist)
    abstract
  end
  
  def with_title(title)
    abstract
  end
  
  def with_artist_and_title(artist, title)
    abstract
  end
end
