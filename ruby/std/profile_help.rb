begin
  require 'ruby-prof'
rescue LoadError
end

module ProfileHelp
  def self.profile(exit_after=false, &block)
    result = RubyProf.profile do
      yield 
    end
    
    printer = RubyProf::FlatPrinter.new(result)
    printer.print($stdout, {:min_percent=>0})
    printer = RubyProf::GraphPrinter.new(result)
    printer.print($stdout, {:min_percent=>0})
    Kernel.exit if exit_after
  end
  
  def self.profile_flat(exit_after=false, &block)
    result = RubyProf.profile do
      yield 
    end
    
    printer = RubyProf::FlatPrinter.new(result)
    printer.print($stdout, {:min_percent=>0})
    Kernel.exit if exit_after
  end
  
  def self.profile_graph(exit_after=false, &block)
    result = RubyProf.profile do
      yield 
    end
    
    printer = RubyProf::GraphPrinter.new(result)
    printer.print($stdout, {:min_percent=>0})
    Kernel.exit if exit_after
  end
end
