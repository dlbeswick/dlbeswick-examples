$: << Dir.pwd

require './Source/djinit.rb'
require './Source/djapp.rb'

def dj_main
  $djApp = DJApp.new($argv_opts)
  $djApp.run
end

if !$post_mortem_debugging
  begin
    dj_main
  rescue RuntimeError => e
    puts "DJ: Runtime Error (#{e.class}):"
  
    error_str = e.to_s
    
    raise "Description of exception of type #{e.class} is too long (#{error_str.length})." if error_str.length > 4096
    $stderr.puts error_str
    $stderr.puts e.backtrace
  
    #raise
  rescue Exception => e
    $stderr.puts 
    $stderr.puts "#{$0}: Exception occurred (#{e.class}.)"
  
    $stderr.puts "#{$0}: #{e}"
    $stderr.puts "#{$0}: #{e.backtrace.join($/)}"
    
    $stderr.puts
    
    begin
      #$djApp.close
    rescue Exception => e2
      $stderr.puts "Error closing app: #{e2} (#{e2.class})"  
    end
    
    #raise e
  end
else
  dj_main
end
