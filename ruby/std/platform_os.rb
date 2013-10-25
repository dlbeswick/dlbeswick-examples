require 'std/platform'
require 'std/algorithm'
require 'std/module_logging'
require 'std/raise'  
require 'tempfile'
require 'rbconfig'

class PlatformOS < Platform
  include ModuleLogging
  include Raise
  
  exception_module_context do name(); end

  def self.autotools_base_path_dirname
    'autotools'
  end

  def self.autotools_base_path
    Algorithm.recurse_from_object(Path.new(Dir.pwd), :parent) do |path|
      test_path = path + autotools_base_path_dirname()
      return test_path if test_path.exists?
    end

    nil
  end

  def self.default_configuration
    result = ENV['PLATFORMOS_CONFIGURATION']
    result ||= 'Debug'
  end
  
  def self.configuration=(string)
    @configuration = string.downcase
  end
  
  def self.configuration
    @configuration
  end
  
  # tbd: make false by default so it is not enabled in program releases
  def self.development?
    true
  end
  
  def self.dylib_prefix
    if msvc?
      ''
    else
      'lib'
    end
  end

  def self.dylib_extension
    if osx?
      'dylib'
    elsif windows?
      'dll'
    elsif linux?
      'so'
    else
      raise "No dylib_extension defined for platform."
    end
  end
  
  def self.dylib_module_paths(module_name, use_config_prefix, use_platform_prefix)
    dylib_filename = if use_platform_prefix
      Path.new("#{dylib_prefix}#{module_name}.#{dylib_extension}")
    else
      Path.new("#{module_name}.#{dylib_extension}")
    end
    
    if use_config_prefix
      native_module_dirs(module_name).collect do |path|
        path + dylib_filename
      end
    else
      [dylib_filename]
    end
  end
  
  def self.dylib_path_env_name
    if osx?
      'DYLD_LIBRARY_PATH'
    elsif unix?
      'LD_LIBRARY_PATH'
    elsif windows?
      env_path_name()
    else
      specific()
    end
  end
  
  def self.framework_path_env_name
    if osx?
      'DYLD_FRAMEWORK_PATH'
    else
      specific()      
    end
  end
  
  def self.framework_path_env_separator
    env_separator()
  end
  
  # potentially built with eclipse
  def self.eclipse?
    linux? || mingw?
  end

  # potentially built with autotools
  def self.autotools?
    linux? || mingw?
  end
  
  def self.env_separator
    if unix?
      ':'
    elsif windows?
      ';'
    else
      specific()
    end
  end
  
  def self.executable_extension
    if unix?
      ''
    elsif windows?
      'exe'
    else
      specific()
    end
  end
  
  def self.dylib_path_env_separator
    env_separator()
  end

  def self.gnome?
    linux? # improve gui platform detection
  end
  
  def self.gui_has_menu_bar_per_window
    # tbd: make this test more correct. osx, or ubuntu unity is in use 
    windows?
  end
  
  def self.extension_module_fileextension
    if osx?
      'bundle'
    else
      'so'
    end
  end
  
  # A list of potential paths to the extension module.
  def self.extension_module_paths(module_path_str, extension_module_postfix='')
    extension_module_dirs(module_path_str, extension_module_postfix).collect do |path|
      Path.to_file(path + Path.to_file(module_path_str).file).with_extension(extension_module_fileextension())
    end
  end
  
  def self.extension_module_dir_need_version_postfix?
    false
  end
  
  # A list of potential base paths for an extension module with the given module name.
  def self.extension_module_dirs(module_path_str, configuration_postfix='')
    ruby_version_postfix = if extension_module_dir_need_version_postfix?
      RUBY_VERSION
    else
      ""
    end
    
    module_name = Path.to_file(module_path_str).file.literal
    
    result = []

    if msvc?
      result << Path.new("#{module_name}/msvc/#{PlatformOS.platform.downcase}/#{PlatformOS.configuration.downcase}#{ruby_version_postfix}#{configuration_postfix}")
    end

    if autotools?
      at_base_path = autotools_base_path()
      if at_base_path
        result << at_base_path + PlatformOS.platform.downcase + PlatformOS.configuration.downcase + "lib" + module_name
      end
    end

    if eclipse?
      result << Path.to_directory("#{module_path_str}/#{PlatformOS.platform.downcase}_#{PlatformOS.configuration.downcase}#{ruby_version_postfix}#{configuration_postfix}")
end

    if xcode?
      result << Path.new("#{module_name}/xcode/build/#{PlatformOS.configuration.downcase}#{ruby_version_postfix}#{configuration_postfix}")
    end

    raise "No native module dir result defined for platform." if result.empty?
    
    result
  end
  
  def self.include_dylib_path(path)
    @dylib_paths << path
  end
  
  def self.dylib_dirs
    $:.collect { |str| Path.new(str) } + @dylib_paths
  end
  
  def self._dylib_paths(module_name, use_config_prefix, use_platform_prefix)
    base_paths = dylib_dirs()
    module_paths = dylib_module_paths(module_name, use_config_prefix, use_platform_prefix)

    result = base_paths.product(module_paths) do |base_path, module_path|
      base_path + module_path
    end

    if msvc?
      # untested
      result += module_paths.product(result) do |module_path, result_path|
        module_path.file + result_path
      end    
    end
    
    result
  end
  
  def self.dylib_paths(module_name, use_config_prefix, use_platform_prefix)
    result = _dylib_paths(module_name, use_config_prefix, use_platform_prefix)
  
    # temporary hack to support Standard_d.dll
    if msvc?
      if configuration() == 'debug'
        debug_result = result.collect do |path|
          path.no_file + Path.new(path.file.no_extension.literal + '_d.dll')
        end
        
        result = debug_result + result
      end
    end
    
    result
  end
  
  def self.env_path_name
    if unix? || windows?
      'PATH'
    else
      specific()
    end
  end
  
  def self.env_paths
    ENV[env_path_name()].split(env_separator()).collect do |path|
      Path.new(path)
    end
  end
  
  def self.framework_path(dir_path, module_name)
    raise "Needs fixing."
    dir_path + native_module_dir(module_name) +"#{module_name}.framework"
  end
  
  def self.framework_paths(module_name)
    dylib_dirs.collect do |dir|
      framework_path(dir, module_name)
    end
  end
  
  def self.require_dylib(module_name, use_config_prefix=true, use_platform_prefix=true)
    if linux?
      log { "#{module_name}: require_dylib not possible on linux, ignoring." }
      return
    end
    
    extra_error = if osx?
      begin
        require_framework(module_name)
        return
      rescue Exception=>e
        e.to_s + "\n\n"
      end
    else
      ''
    end
    
    paths = dylib_paths(module_name, use_config_prefix, use_platform_prefix) 
    paths.each do |libpath|
      if libpath.exists?
        log { "#{libpath}" }
        ENV[dylib_path_env_name()] ||= ""
        ENV[dylib_path_env_name()] += "#{dylib_path_env_separator}" if !ENV[dylib_path_env_name()].empty?
        ENV[dylib_path_env_name()] += "#{libpath.no_file.to_sys}"

        return
      end
    end
    
    raise "No dynamic library found:\n\n#{extra_error}#{dylib_module_paths(module_name, use_config_prefix, use_platform_prefix).collect{ |path| path.file }.join(',')} found in any platform dylib or ruby library path:\n#{paths.join("  \n")}"
  end
  
  # this method doesn't work. you can't set this environment var while the program is already running. must fix.
  def self.require_framework(module_name)
    paths = framework_paths(module_name)
     
    paths.each do |path|
      if path.exists?
        log { "#{path}" }
        ENV[framework_path_env_name()] ||= ""
        ENV[framework_path_env_name()] += "#{framework_path_env_separator}" if !ENV[framework_path_env_name()].empty?
        ENV[framework_path_env_name()] += "#{path.parent.to_sys}"

        note { "#{framework_path_env_name()}: #{ENV[framework_path_env_name()]}" }
        return
      end
    end
    
    raise "No framework '#{module_name}' found in any platform dylib or ruby library path:\n#{paths.join("  \n")}"
  end
  
  def self.require(module_path_str, configuration_postfix='')
    module_name = Path.to_file(module_path_str).file.literal

    potential_require_paths = extension_module_paths(module_path_str, configuration_postfix)

    require_statement = potential_require_paths.find do |path|
      path.exists?
    end

    errors = []

    potential_require_paths.each do |path|
      require_statement = path.no_extension.literal

      begin
        Kernel.require require_statement
      rescue LoadError=>e
        errors << e
      end
    
      if errors.empty?
        log { require_statement }
        break
      end
    end

    if !errors.empty?
      err { "Errors while requiring '#{module_path_str}':\n   #{errors.join($/+'  ')}" }
      found = false
      
      # Append [] for absolute paths
      potential_require_paths.each do |require_path|
        test_paths = if require_path.absolute?
          [require_path]
        else
                       $:.collect { |libpath| Path.to_directory(libpath) + Path.to_file(require_path) }
        end

        test_paths.each do |module_path|
          if module_path.exists?
            found = true
            log { "Note: Module exists at #{module_path}" }
            
            if osx?
              log { "Dependencies: (executing otool)" }
              log { `otool -L #{module_path}` }
            end
          end
        end
      end
      
      if !found
        raise "The module was not found in any library path:\n  " + $:.join("\n  ") + "\nPotentials: \n" + potential_require_paths.join("\n ")
      else
        raise "Require failed, missing dependent lib? #{dylib_path_env_name()}: #{ENV[dylib_path_env_name()]}"
      end
    end
  end  
  
  def self.run_ruby(*args)
    run_ruby_internal(args)
  end
  
  def self.run_ruby_cmd_window(*args)
    run_ruby_internal(args, true, false)
  end
    
  def self.run_ruby_cmd_window_persist(*args)
    run_ruby_internal(args, true, true)
  end
  
  def self.run_ruby_internal(args, cmd_window = false, keep_window = false)
    cmd_line = "#{ruby_path().to_sys} #{args.join(' ')}"

    pid = if windows?
      if !PlatformRuby.v19?
        Object.instance_eval do require 'win32/process'; end
          
        startup_info = {}#{:stderr=>2}
         
        log { "#{cmd_line}" }
        Process.create({:app_name=>cmd_line, :startup_info=>startup_info})
      else
        if cmd_window
          command_param = if keep_window
            '/k'
          else
            '/c'
          end
                      
          cmd_line = "start \"\" cmd.exe #{command_param} " + cmd_line
        end
      
        log { "#{cmd_line}" }
        spawn(cmd_line)
      end
    else
      if cmd_window
        if PlatformOS.osx?
          command_param = if keep_window
            ''
          else
            ''
          end
                      
          tf = Tempfile.open('w')
          
          ENV.each do |k,v|
            tf.write("echo #{k}=#{v};export #{k}=#{v}\n")  
          end
          
          tf.write("cd #{Dir.pwd};#{command_param}#{cmd_line}")
          tf.close
          
          File.chmod(0766, tf.path)
          
          cmd_line = "open -a /Applications/Utilities/Terminal.app #{tf.path}"
          log { "#{cmd_line}" }
          system(cmd_line)
        else
          if PlatformRuby.v19?
            spawn(cmd_line)
          else
            raise "Not implemented."
          end
        end
      else
        log { "#{cmd_line}" }
        spawn(cmd_line)
      end
    end
    
    pid
  end
  
  def self.shell_cmdline_format_path(path_string)
    "\"#{path_string}\""
  end
  
  # Performs the native shell environments 'open' command, which is the command that executes the
  # os's default file association program for the given file at "path." This is 'open' on osx,
  # 'start' on windows, 'gnome-open' on gnome.
  def self.shell_open(path)
    if windows?
      system "cmd /c start \"\" #{path.to_cmdline}"
    elsif linux?
      if gnome?
        system "gnome-open #{path.to_cmdline}"
      else
        raise "Not implemented."
      end
    else
      raise "Not implemented."
    end
  end
    
  def self.mingw?
    RUBY_VERSION >= "1.9.1"
  end
    
  def self.msvc?
    self.platform == 'Windows' && !mingw?
  end
  
  def self.windows?
    self.platform == 'Windows'
  end

  def self.osx?
    self.platform == 'OSX'
  end
  
  def self.unix?
    !windows?
  end

  # built with xcode
  def self.xcode?
    osx?
  end
    
  def self.filenameEncoding
    @filenameEncoding
  end

  def self.calculate_platform
    detectPlatform()
  end
  
  def self.detectPlatform
    if RUBY_PLATFORM =~ /darwin/
      @platform = 'OSX'
      @filenameEncoding = 'utf8-mac'
    elsif RUBY_PLATFORM =~ /win32/ || RUBY_PLATFORM =~ /mingw32/
      @platform = 'Windows'
      @filenameEncoding = 'Windows-1252'
    elsif RUBY_PLATFORM =~ /linux/
      @platform = 'Linux'
      # This was probably set because auanalysis write to a windows disk, and should probably be set to
      # UTF-8
      @filenameEncoding = 'Windows-1252'
    else
      raise "Cannot detect platform. ('#{RUBY_PLATFORM}')"
    end
  end
  
  self.configuration = default_configuration()
  @dylib_paths = []
    
  detectPlatform
  
:protected
  def self.ruby_path
    result = Path.new(Config::CONFIG['bindir']).with_file(Config::CONFIG['ruby_install_name'])
    if Config::CONFIG['EXEEXT'] && !Config::CONFIG['EXEEXT'].empty?
      result.extension = Config::CONFIG['EXEEXT'][1..-1]
    end
    result
  end
end

require 'std/path' if !Object.const_defined?(:Path)

if PlatformOS.development? && Object.const_defined?(:Path)
  PlatformOS.include_dylib_path(Path.new('../../Common').absolute)
  PlatformOS.include_dylib_path(Path.new('../../../Common').absolute)
end

module PlatformOSSpecific
  include PlatformSpecific
end
