require 'progress'

class ProgressGUI < Progress
  def initialize(total = nil, description = nil, &block)
    
  end
end

class ProgressGUIHandler < ProgressDelayed
end