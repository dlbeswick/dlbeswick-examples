require 'std/abstract'
require 'std/platform'

class PlatformGUI < Platform
  include Abstract
  
  self.platform = 'Wx'

  def self.gtk?
    false
  end
end

module PlatformGUISpecific
  include PlatformSpecific
  
  def self.platform_class
    PlatformGUI
  end
end
