require 'optparse'

$: << "."

#ENV['LD_DEBUG'] = 'files'

def xx(what)
  puts "#{caller.first}: #{what.inspect}"
end

class OptionParserToHash < OptionParser
  def initialize(hash)
    @hash = hash
    super()
  end
  
  def to_hash(option_symbol, *args)
    raise "Option #{option_symbol} already defined." if @hash.has_key?(option_symbol)
    
    on(*args) do |value|
      @hash[option_symbol] = value 
    end
  end
end
  
def init_options
  $options = {}
    
  opts = OptionParserToHash.new($options) do |opts|
    opts.banner = "Usage: auanalysis.rb [options]"
 
    opts.to_hash(
      :configuration, 
      "--configuration CONFIG_NAME",
      "Configuration to use for native libraries."
    )
 
    opts.to_hash(
      :platform, 
      "--platform PLATFORM_NAME",
      "Override the automatic detection of the platform to use for native libraries."
    )

    opts.to_hash(
      :test, 
      "--test",
      "Initiate test mode."
    )

    opts.to_hash(
      :task, 
      "--task TASK_CLASS_NAME",
      "Run the given task. Read task parameters from stdin."
    )

    opts.on("-h", "--help", "Show usage information.") do
      puts opts
      exit
    end
  end

  opts.parse(ARGV)

  if $options[:configuration] != nil
    PlatformOS.configuration = $options[:configuration]
  end
end

init_options()

def execute_task(task_class_name)
  program_output_stream = $stdout
#  $stdout = File.new('task.out', 'w'); $stderr.reopen($stdout) 
  $stdout = $stderr # avoid 'puts' etc contaminating output

  result = 
    begin
      require 'std/EnumerableSubclasses'
      EnumerableSubclasses.require('./transform/export/task_*')
  
      task_class = Object.const_get(task_class_name)
      
      task = task_class.new(*Marshal.load($stdin.read))
      task.run_begin
      task.run
      task.run_end
      
      [true, task.result]
     rescue Exception => e
      $stderr.puts e
      $stderr.puts e.backtrace
      [false, e.message, e.backtrace]
     end

  program_output_stream.write(Marshal.dump(result))

  exit_code =
    if result.first == true
      0
    else
      1
    end

  exit(exit_code)
end

if $options[:task]
  execute_task($options[:task])
  # Note: Execute_task stops execution.
end

#$DEBUG = true
begin
  require 'debugger'
rescue
end

$stdout.sync = true
require 'std/platform_os'

require 'narray_auanalysis'

require 'std/file_memory_mapped'

PlatformOS.require_dylib 'Standard'
#if PlatformOS.windows?
#  PlatformOS.require('dsound', '_nomp3')
#else
  #syscall 24
  PlatformOS.require('std/dsound')
#end

require 'std/GUIWx'

require 'std/profile_help'
require 'std/path'
require 'std/platform'
require 'std/view/View'
require 'std/view/ViewContainer'

# tbd: remove
require 'std/view/path'

EnumerableSubclasses.require('view')

require 'transform/transform'
require 'std/raise'
require 'sound_file_audacity'
require 'app'

SoundFileAudacity::FromXML.log = 2

require 'std/interval'

#Progress.default_progress_class=ProgressDummy

def test_transform
  path = Path.new('d:/data/recordings/Natalie Cole/Good To Be Back/A.aup')
  sound_file = SoundFileAudacity.new(path.to_file, path)
  sound_file.cache_max_length_bytes = 0

  flac = Flac.new
  flac.resampler.dither = false
  
  reader = sound_file.sequential_reader

  t = Time.now
    
  DLB::Tempfile.open("tmp", Path.new('.').absolute.to_sys, '', true) do |tf|
    Progress.run(reader.length_bytes, "Write #{tf.path}") do |progress|
      loop do
        # tbd: fix for stereo
        data = reader.read(2**20 / reader.sound_file.bytes_per_sample)
        break if !data
        data = data[0].to_s
        tf.write(data)
        progress << data.length
        #break if Time.now - t > 5
      end
    end
    
    tf.close
    
    output = Path.new('test.flac')
    output.delete if output.exists?
    
    flac.execute(tf.path, output)
  end
end

#test_transform()
#exit

def wavelet_view(sound_file, main_window, wavelet)
  $window_start_s = 1124
  $window_width = 2048

  container = ViewContainers::Single.new(AttributeReference::Object.new(wavelet), main_window, ViewSoundFileWaveletAmplitude_dB_Abs)
  container.view.view_domain_start = sound_file.seconds_to_samples($window_start_s)
  container.view.view_domain_size = $window_width
  container.show
end

def analysis_view(sound_file, main_window, wavelet)
  gap_sample_size = 2048
  gap_threshold_db = -72.0
  gap_threshold_trigger_prop = 0.5
  
  $window_start = 47448042
  $window_width = 107640.298237875

  ta = ThresholdAnalysis.new(sound_file, 0.5, 0.5, gap_sample_size, gap_threshold_db, gap_threshold_trigger_prop)
  container = ViewContainers::Single.new(AttributeReference::Object.new(ta), main_window, GraphViewDiscreet, false)
  container.show
end

#ViewAllAttributesCommon.log=true
#Viewable.log=true
#ViewBase.log=true
#Unique.log=true
#UniqueID.log=true
#UniqueIDConfig.log=true

class ProfileTest
  include Viewable

  @@num = 200
    
  for i in (1..@@num) do
    eval("attr_accessor :_#{i}")
  end
  
  def initialize
    for i in (1..@@num) do
      instance_variable_set("@_#{i}", i.to_s)
    end
  end
end

if true
  $app = App.new
  $app.load

  profile = false
  
  GUI.start do |main_window|
    app_view = ViewContainers::Single.new($app, main_window).view
    #RubyProf.start if profile
    #ViewContainers::Single.new(Array.new(2, ProfileTest.new), app_view)
    #app_view.test_track_boundaries
    #app_view.test_wavelet
    #app_view.test_wavelet_thresholding
  end

  if profile
    result = RubyProf.stop
  
    #printer = RubyProf::FlatPrinter.new(result)
    #printer.print(STDOUT, {})#{:min_percent=>10})
    #printer = RubyProf::GraphPrinter.new(result)
    #printer.print(STDOUT, {})#{:min_percent=>10})
    printer = RubyProf::CallTreePrinter.new(result)
    calltreeout = File.new('calltree.out', 'wb')
    printer.print(calltreeout, {})#{:min_percent=>10})
    calltreeout.close  
    Kernel.exit
  end
else
  puts 'Loading file...'
  sound_file = SoundFile.from_path(Path.new(ARGV[0]))
  sound_file.cache_max_length_bytes = 0
  
  puts 'Loaded.'
  
  GUI.start do |main_window|
    main_window.move(0, 750) if Platform.windows?
    
    wavelet = WaveletDiscreteHaar.new(sound_file, 2)
    
    wavelet_view(sound_file, main_window, wavelet)
    #analysis_view(sound_file, main_window, wavelet)
  
    start = if !$window_start
      sound_file.seconds_to_samples($window_start_s)
    else
      $window_start
    end
   
    container = ViewContainers::Single.new(AttributeReference::Object.new(sound_file), main_window, ViewSoundFileAmplitude, false)
    #container = ViewContainers::Single.new(AttributeReference::Object.new(Threshold_dB.new(sound_file)), main_window, ViewSoundFileAmplitude, false)
    container.view.view_domain_start = start
    container.view.view_domain_size = $window_width
    container.show
    
    #container = ViewContainers::Single.new(AttributeReference::Object.new(wavelet), main_window, ViewSoundFileFrequencyWavelet)
    #container = ViewContainers::Single.new(AttributeReference::Object.new(Maxima2D.new(wavelet)), main_window, ViewSoundFileMaxima)
    #container = ViewContainers::Single.new(AttributeReference::Object.new(wavelet), main_window, ViewSoundFileWaveletAmplitude_dB)
    ##container = ViewContainers::Single.new(AttributeReference::Object.new(Threshold_dB_2D.new(wavelet,gap_threshold_db)), main_window, ViewSoundFileWaveletAmplitude)
    #container.view.view_domain_start = start
    #container.view.view_domain_size = window_width
    #container = ViewContainers::Single.new(AttributeReference::Object.new(Autocorrelation.new(wavelet)), main_window, ViewSoundFileWaveletAmplitude)
  end
end
