def cmdline(key, default = '')
	m = /#{key}=(.*?)($| )/.match(ARGV.join(' '))
	if m
		m[1]
	else
		default
	end
end

module Std
	# ask the user a yes or no question
	def Std.ask(message, options = ['y','n'])
		puts "#{message} (#{options.join('/')})"
	
		input = STDIN.gets
		return true if /#{options[0]}/.match(input)
	end
end