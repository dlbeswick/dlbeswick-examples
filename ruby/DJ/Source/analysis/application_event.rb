require 'std/ConfigYAML'

class ApplicationEvent
  include ConfigYAML
  
  persistent_accessor :time
  
  def initialize
    @time = Time.now
  end
end