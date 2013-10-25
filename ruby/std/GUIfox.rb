require 'platform'
require 'fox16'
require 'fox16/colors'

if $VERBOSE
  $VERBOSE = nil
  require "fox16"
  $VERBOSE = true
else
  $VERBOSE = nil
  require "fox16"
  $VERBOSE = false
end

include Fox

PlatformGUI.name = 'Fox' 

module GUI
  class Layout < FXPacker
  end
end