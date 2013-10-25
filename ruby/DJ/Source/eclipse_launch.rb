$: << '../../std'
require 'platform'

if PlatformOS.osx?
  $config = ARGV[1]
  $config = 'debug' if !$config
  
  exePath = File.join(Dir.pwd, "native/OSX/build/#{$config}/DJ.app")
  
  ENV['DYLD_FRAMEWORK_PATH'] ||= ''
  ENV['DYLD_FRAMEWORK_PATH'] = ENV['DYLD_FRAMEWORK_PATH'] + ":#{exePath}/Contents/Frameworks"
  system "#{exePath}/Contents/MacOS/DJ debug"
else
  system "dj_d.exe debug"
end