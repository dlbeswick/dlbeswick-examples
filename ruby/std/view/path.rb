require 'std/view/View'
require 'std/view/AttributeReferenceTextField'

class Path
  # if ConfigYAML is not included here, paths will not serialize properly.
  # viewable_dependants may be serialized for paths.
  # basically, ConfigYAML must be included in Viewable objects to ensure that variable is not serialized.
  # however ConfigYAML should not be included for basic types like integer, string etc.
  # find a better way to automatically include ConfigYAML in Viewable classes.
  include ConfigYAML
  include Viewable
  
  persistent :segments
  persistent :is_directory
  
  default :segments, []
  default :is_directory, nil
  
  def self.config_version
    1
  end

  def self.config_upgrade(new, old, old_version)
    if old_version < 1
      new.path = Path.split_string(old.instance_variable_get(:@path))
    end
  end
end

class ViewPath < Views::AttributeReferenceTextField
  suits(Path)

  def make_controls
    super
    layoutAddHorizontal do 
      makeButton(self, "Browse") do
        show_browser_and_set_result()
      end
    end
  end
    
  def value_from_s(text)
    Path.new(text)
  end

  def targetValue
    target().literal
  end
  
  def self.order
    -0.8
  end
  
protected
  def browser_title
    "Choose path: #{attribute().viewable_description}"
  end
  
  def show_browser_and_set_result
    result = if target() && target().directory?
      GUI.user_get_dir(self, browser_title(), target())
    else
      GUI.user_get_file(self, browser_title(), target())
    end
    
    if result
      attribute().set(result)
      attribute().modified(self)
    end
  end
end
