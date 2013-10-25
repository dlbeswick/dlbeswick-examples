Encoding.default_internal = 'UTF-8'

require 'rubygems'
require 'debugger'
require 'trollop'

$argv_orig = ARGV.dup

$argv_opts = Trollop::options do
  opt :config, "Specify a build configuration (Debug, Release).", :default => 'Debug'
  opt :playlist, "Add a playlist file to the music collection.", :type => :strings
  opt :server, "The address of the drb server (if running the GUI.)", :type => :string
  opt :run_server, "Run a drb server if one hasn't already been started.", :default => false
  opt :run_gui, "Run the gui process automatically (if running the server.)", :default => false
end

require 'std/platform_os'

require 'drb'
require './Source/drb_hacks'

$debug_ruby19_mingw = true && PlatformRuby.v19? && PlatformOS.windows?

PlatformOS.log=3

def xx(what)
  puts "(#{$0})#{caller.first}: #{what.inspect}"
end

def xx_outline_convert(what)
  if what.kind_of?(Array)
    outlined = what.collect { |e| xx_outline_convert(e) }
    "[#{outlined.join(", ")}]"
  else
    "<#{what.class}>"
  end
end

def xx_outline(info, what)
  puts "(#{$0})#{caller.first}: #{info}, #{xx_outline_convert(what)}"
end

def xxe(what)
  $stderr.puts "(#{$0})#{caller.first}: #{what.inspect}"
end

# ensure ruby standard library path points to app's standard ruby library
$: << '../../std'
$: << 'std'

if PlatformOS.windows?
  $: << 'd:/depot/projects/ruby/dj/source'
end

PlatformOS.configuration = $argv_opts[:config]
 
puts "DJ init."
puts "- Configuration is #{PlatformOS.configuration}."
puts "- Platform is #{PlatformOS.platform}."

if PlatformOS.osx?
  # for x11 support
  ENV['DISPLAY']=':0' if ENV['DISPLAY'] && ENV['DISPLAY'].empty?

  puts "- OSX Framework path is #{ENV['DYLD_FRAMEWORK_PATH']}"
  puts "- OSX Fallback framework path is #{ENV['DYLD_FALLBACK_FRAMEWORK_PATH']}"
end

puts

#require 'ruby-debug'
if Object.const_defined?(:Debugger)
  #$post_mortem_debugging = true
  #Debugger.post_mortem
end

$stdout.sync = true
$stderr.sync = true

require 'std/platform'
require 'std/SafeThread'
require 'mutex_m'

$fatalExceptions = false

# logging
require 'logger'

$log = Logger.new(STDOUT)
$log.progname = 'DJ'
$warn = Logger.new(STDERR)
$warn.progname = 'DJ'
$err = Logger.new(STDERR)
$err.progname = 'DJ'

if PlatformOS.development? && PlatformOS.mingw?
  $: << "c:/mingw/bin"
end

# all SWIG modules must be initialised here
PlatformOS.require_dylib('standard')
if PlatformOS.mingw?
  #PlatformOS.require_dylib('lame_enc', false, false) # tbd: find out why lame_enc is getting linked without .dll extension
  PlatformOS.require_dylib('mp3lame-0', false)
end
PlatformOS.require "std/mediatag"
PlatformOS.require "std/midiinterface"
PlatformOS.require "std/dsound"
PlatformOS.require "std/timer"

class Dsound::Gain
  yaml_as "tag:david,2011:dsound_gain"
  
  def to_yaml( opts = {}, is_cloning = false )
    YAML::quick_emit( nil, opts ) do |out|
      out.map( taguri, to_yaml_style ) do |map|
        map.add('db', db())
      end
    end
  end
  
  def self.yaml_new(klass, tag, val)
    self.new(val['db'])
  end
  
  def _dump(depth)
    Marshal.dump(db().to_f)
  end
  
  def self._load(str)
    db_val = Marshal.load(str)
    self.new(db_val)
  end
  
  def to_s
    db().to_s
  end
end
  
# Standard library
require 'std/algorithm'
require 'std/mapping'
require 'std/MappingPersistent'
require 'std/MappingSymbolDefaults'
require 'std/module_logging'
require 'std/path'
require 'std/UniqueIDConfig'
require 'std/unique'

# Internal
require 'std/AttributeReference'

require './Source/ChangeActuator'
require './Source/midiactuator'
require './Source/MIDIController'
require './Source/sourcemodifier'

require './Source/test'

module Mapping
  class Base
    include DRb::DRbUndumped
  end
end

module Config
  def Config.basePath
    $djApp.configFilePath
  end
end

class View
protected
  def self.send_authoritive(force_keep_result_reference, instance, symbol, *args, &block)
    DRb.send_authoritive(force_keep_result_reference, instance, symbol, *args, &block)
  end
  
  def self.prevent_gc_authoritive(object)
    DRb.prevent_gc_authoritive(object)
  end

  def self.release_authoritive(instance)
    DRb.release_authoritive(instance)
  end
end

