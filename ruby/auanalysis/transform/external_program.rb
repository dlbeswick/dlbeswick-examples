require 'std/abstract'
require 'std/path'
require 'std/platform_os'
require 'std/SymbolDefaults'
require 'std/view/Viewable'

class ExternalProgram
  include Abstract
  include ConfigYAML
  include Raise
  include SymbolDefaults
  include Viewable
  
  persistent_infer :path do
    # not cached because it's not very expensive, and maybe the user will like to drop tools into
    # "program files" while the program is running, like i did.
    path = find_path_if_possible()
    
    if !path
      raise "No path was given for tool '#{name()}', and it was not found in the following paths:\n#{search_paths().join("\t\n")}"
    end
    
    path
  end
  
  def execute(input_paths, output_path)
    parameters = parameters(Array(input_paths), output_path)
    parameters = process_parameters(parameters)
    
    command = "\"#{path().to_sys}\" #{parameters} 2>&1" 
    
    result = Progress.run(0, command) do |progress|
      `#{command}`
    end
    
    result ||= []  
    
    if $? != 0
      $stderr.puts result
      raise "Command failed with error code #{$?} (#{command})."
    else
      $stdout.puts result
    end
    
    result
  end
  
  def execute_pipe(pipe_data_io, output_io = $stdout, progress = nil)
    raise 'Not tested.'
    
    path = path()
    
    if !path
      raise "Could not locate program '#{name()}'. Search paths:\n[#{search_paths().join("\n")}]\n"
    end
    
    IO.popen("\"#{path.to_sys}\" #{parameters()} 2>&1", 'wb') do |io|
      loop do
        data = pipe_data_io.read(65536)
        break if !data
        io.write(data)
        progress << data.length if progress
      end
    end
  end
  
  def filename
    Path.new(name()).with_extension(PlatformOS.executable_extension)
  end
  
  def find_path_if_possible
    search_paths().each do |path|
      if path.exists?
        return path
      end
    end

    nil    
  end
  
  def name
    self.class.name.downcase
  end
  
  def parameters(input_paths, output_path)
    abstract
  end
  
  def search_dirs
    result = PlatformOS.env_paths
      
    if PlatformOS.windows?
      result << Path.new(ENV['ProgramFiles'])
      result << Path.new(ENV['ProgramFiles(x86)']) if ENV['ProgramFiles(x86)']
        
      result.collect! { |path| path + search_path_dir() }
    end
    
    result
  end
  
  def search_paths
    search_dirs().collect { |path| path.with_file(filename().literal) }
  end
  
  def search_path_dir
    name()
  end
  
  def to_s
    "#{name}: #{parameters(Array(Path.new('input')), Path.new('output')).join(' ')}"
  end
  
protected
  def process_parameter_value(value)
    if value.kind_of?(Path)
      "\"#{value.to_sys}\""
    elsif value.kind_of?(Array)
      if !value.empty?
        value.first.to_s
      else
        ''
      end
    else
      value.to_s
    end
  end
  
  def process_parameters(parameters)
    if parameters.kind_of?(String)
      parameters
    elsif parameters.kind_of?(Array)
      result = ""
      
      parameters.each_with_index do |parameter, i|
        if parameter.kind_of?(Array)
          if parameter.length == 1
            result << process_parameter_value(parameter) 
          elsif parameter.length == 2
            result << parameter[0].to_s 
            result << process_parameter_value(parameter[1]) 
          else
            raise "Bad parameter, must be a string or array with less than 2 elements (#{parameter.inspect})" if parameter.length > 2
          end
        else
          result << process_parameter_value(parameter)
        end
        
        if i < parameters.length - 1
          result << " "
        end
      end
      
      result
    end
  end
end
