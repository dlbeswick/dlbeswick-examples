module Test
  class ActuatorNotify
    def self.test
      if @thread
        @thread.exit
        @thread = nil
        return
      end 
      
      @thread = SafeThread.new do 
        loop do
          SourceModifier.instances.each do |modifier|
            modifier.set_value(Mapping.linear(rand, 0.0, 1.0, modifier.min, modifier.max), self)
          end
          sleep(0.1)
        end while !SafeThread.exit?
      end 
    end
  end 
end