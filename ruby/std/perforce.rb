require 'ftools'

class Perforce
	class Error < RuntimeError
		def self.error?(cmdResultCode)
			cmdResultCode != 0
		end
		
		def initialize(cmd, cmdResult)
			@cmd = cmd
			@errors = cmdResult.scan(/^error\:.+$/)
			@result = cmdResult
		end
		
		def message
			'P4 cmd: ' << @cmd << "\n\nP4 errors: " << @errors.join($/) << $/ << 'Output: ' << @result
		end
		
		def to_s
			message
		end

		def result
			@result
		end
	end

	@@echo = false

	def Perforce.echo
		@@echo
	end
	
	def Perforce.echo=(val)
		@@echo = val
	end

	def Perforce.p4io(args, change = nil)
		match = /(\w+)(.*)/.match(args)
		openStr = 'p4 '
		
		if change && change != P4Change.default then
			openStr << (match[1] + " -c #{change.number} " + match[2])
		else
			openStr << match[1]
			openStr << (' ' + match[2]) if match.length > 2
		end
		
		openStr << ' 2>&1'
		
		puts "EXEC #{openStr}" if @@echo
		
		io = IO.popen(openStr, 'r+')
		raise("p4 command failed") if io == nil
		io
	end
	
	def Perforce.p4(args, change = nil)
		io = p4io(args, change)
		a = io.read
		io.close
		
		puts a if @@echo
		
		raise Error.new("p4 #{args}", a) if Error.error?($?)
		
		a
	end

	def Perforce.opened(change = nil)
		cmd = p4('opened', change).split($/)
		
		return [] if cmd[0] =~ /File\(s\) not opened on this client./
		
		result = []
		cmd.each do |l|
			l =~ /^(.*)#/
			result << $1
		end
		
		result
	end
	
# returns true if files are open on client
	def Perforce.opened?(change = nil)
		return Perforce.opened(change) != []
	end
	
# returns most recent changelists
	def Perforce.processChanges(text)
		splitText = text.scan(/^(Change.+?)(?=(?=Change)|\z)/m)

		changes = []
		splitText.each do |e|
			changes << P4Change.fromSpec(e[0]) 
		end
		
		return changes
	end
	
	def Perforce.recentChanges(num = 1)
		return processChanges(p4("changes -m #{num} -l"))
	end
	
	# returns a string of single or array of filespecs surrounded by quotes
	def Perforce.formatFilespec(*fileSpecs)
		if !fileSpecs.empty?
			newSpecs = fileSpecs.collect do |f|
				if !f.include?("\"")
					"\"#{f}\""
				else
					f
				end
			end
		end
	
		newSpecs.join(' ')
	end
	
# returns an array of changes in the given range.
# only includes changes affecting files matching 'filterSpecs', if given.
	def Perforce.changes(startNum = nil, endNum = nil, status = 'submitted', filterSpecs = [])
		formatFilespec(filterSpecs)
	
		cmd = "changes -s #{status} -l #{formatFilespec(filterSpecs)}"
		cmd += " @#{startNum}" if startNum
		if endNum
			cmd += ",#{endNum}" 
		else
			cmd += ',#head'
		end
		return processChanges(p4(cmd))
	end
	
# return false if changes are open, not including default changelist
	def Perforce.changes?(status = 'pending')
		!changes.empty?
	end
	
	def Perforce.add(fileSpec, change)
		p4("add \"#{fileSpec}\"", change)
	end

	def Perforce.edit(fileSpec, change)
		cmd = p4("edit \"#{fileSpec}\"", change)
		if /not on client\./m.match(cmd) != nil
			raise "Perforce.edit: #{cmd}"
		end
	end

	def Perforce.delete(fileSpec, change)
		p4("delete \"#{fileSpec}\"", change)
	end

	def Perforce.revert(fileSpec, change)
		p4("revert \"#{fileSpec}\"", change)
	end

	def Perforce.submit(change)
		p4("submit", change)
	end

	def Perforce.files(fileRevRange)
		p4("files \"#{fileRevRange}\"")
		cmd.scan(/(\/\/.+?)\#/)
	end

# revert unchanged files
	def Perforce.revertUnchanged(change)
		p4("revert -a", change)
	end
	
# autoresolve files
	def Perforce.resolve(fileSpec = nil)
		if fileSpec != nil then
			p4("resolve -am \"#{fileSpec}\"")
		else
			p4("resolve -am")
		end
		
		return Perforce.resolved?
	end

# return false if files unresolved
	def Perforce.resolved?
		p4("resolve -n")
		
		return (cmd =~ /No file\(s\) to resolve./) != nil
	end

# perform integrations
	def Perforce.integrate(srcFileSpec, dstFileSpec, change, baseless = false, reverse = false, deletes = false)
		cmdString = 'integrate'
		cmdString << ' -I' if baseless
		cmdString << ' -r' if reverse
		cmdString << ' -Ds' if deletes
		cmdString << "\"#{srcFileSpec}\" \"#{dstFileSpec}\""
		p4(cmdString, change)
	end

	def Perforce.integrate(branchSpec, change, baseless = false, reverse = false, deletes = false)
		cmdString = 'integrate'
		cmdString << ' -I' if baseless
		cmdString << ' -r' if reverse
		cmdString << ' -Ds' if deletes
		cmdString << " -b \"#{branchSpec}\""
		p4(cmdString, change)
	end

# branches
# returns array describing the branch view
	def Perforce.branch?(branchSpec)
		# not recently tested, may not work.
		cmdResult = p4("branch -o \"#{branchSpec}\"")
		re = /^View\:\n(.*)/m.match(cmdResult)
		return nil if !re || re.length < 2
		
		re[1].split("\n\t")
	end

# manual integration from a non-source-controlled source
# both repositories must have identical directory structures
	def Perforce.manualIntegration(localRepositoryRootPath, uncontrolledPath, changeDesc)
		if !opened.empty?
			puts 'All changelists must be submitted before performing a manual integration'
			return
		end
		
		newChange = P4Change.new(changeDesc)
		
		oldDir = Dir.getwd
		
		recurseUCD = proc { |curPath, local|
			dir = Dir.new(curPath)
			dir.each do |d|
				next if d == '.' || d == '..'
				
				remoteFile = curPath + '/' + d
				localFile = local + '/' + d
				if File.stat(remoteFile).directory?
					recurseUCD.call(remoteFile, localFile) 
				else
					
					if !FileTest.exist?(localFile) then
						puts remoteFile + " ->+ " + localFile
						File.makedirs(File.dirname(localFile))
						File.syscopy(remoteFile, localFile)
						Perforce.add(localFile, newChange)
					else
						puts remoteFile + " -> " + localFile
						Perforce.edit(localFile, newChange)
						File.syscopy(remoteFile, localFile)
					end

				end
			end
		}
		
		Dir.chdir(uncontrolledPath)
		recurseUCD.call('.', localRepositoryRootPath)
		
		Perforce.revert(newChange)
	end
	
	# Returns depot path, workspace path, filesystem path given a P4Filespec object, Path object, or to_s responding object. 
	def Perforce.where(localPathOrFilespec)
		input = if localPathOrFilespec.respond_to?(:to_p4)
			localPathOrFilespec.to_p4
		elsif localPathOrFilespec.respond_to?(:to_sys)
			localPathOrFilespec.to_sys
		else
			localPathOrFilespec.to_s
		end
		
		result = Perforce.p4("where \"#{input}\"")
		
		depot = /^(.+?) \/\//.match(result)[1]
		client = /^.+? (\/\/.+?) .+?\:/.match(result)[1]
		local = /^.+? \/\/.+? (.+?\:.+$)/.match(result)[1]
		
		return depot, client, local
	end
end


class P4Change
	attr_accessor :number
	attr_accessor :description
	
	def self.finalizer(number)
		proc do
			# revert change if still open
			result = Perforce.p4('changes -s pending')
			if /Change #{number}/m.match(result) != nil
				puts "Change \##{number} finalized while still pending, reverting and deleting."
				begin
					begin
						Perforce.p4("revert -c #{number} //...")
					rescue Perforce::Error
					end
					
					Perforce.p4("change -d #{number}")
				rescue Exception=>e
					puts "Error while deleting changelist:"
					puts e.message
				end
			end
		end
	end
	
	def initialize(description)
		return if description == nil
		
		@description = description
	
		io = Perforce.p4io('change -i') << "Change:new\nStatus:new\nDescription:#{@description}"
		io.close_write
		result = io.readline
		
		puts result if Perforce.echo

		m = /Change (.+) created./.match(result)
		raise("Error creating change (#{description}), result was #{result}") if !m
		@number = m[1].to_i
		
		ObjectSpace.define_finalizer(self, self.class.finalizer(@number))
	end

	def opened
		Perforce.opened(self)
	end
	
	def opened?
		Perforce.opened?(self)
	end

	def add(fileSpec)
		Perforce.add(fileSpec, self)
	end

	def edit(fileSpec)
		Perforce.edit(fileSpec, self)
	end

	def revert(fileSpec = "//...")
		Perforce.revert(fileSpec, self)
	end

	def revertUnchanged
		Perforce.revertUnchanged(self)
	end

	def delete(fileSpec)
		Perforce.delete(fileSpec, self)
	end
	
	def kill
		raise "Can't kill non-open changelist #{@number}" if !Perforce.opened(self).empty?
		Perforce.p4("change -d #{@number}")
	end
	
	def submit
		revertUnchanged
		raise "No files to submit." if !opened?
		Perforce.submit(self)
	end
	
	def P4Change.fromSpec(spec)
		m = /^Change (\d+).+?$(.+)/m.match(spec)
		raise "Bad changespec: #{spec}" if !m
		number = m[1]
		description = m[2]
		c = P4Change.new(nil)
		c.number = number.to_i
		c.description = description
		return c
	end
	
	def to_s
		return "Change #{@number}\n#{@description}"
	end
	
	@@default = P4Change.new(nil)
	@@default.number = -1
	def P4Change.default
		return @@default
	end
end

class P4Filespec
	def initialize(pathObj='')
		if !pathObj.empty?
			depot, client, local = Perforce.where(pathObj)
			@fileSpec = depot
		else
			@fileSpec = ''
		end
	end
	
	def clone
		new = P4Filespec.new
		new.set(@fileSpec.dup)
		new
	end
	
	def +(fileSpec)
		ret = self.clone
	  ret << fileSpec
		return ret
	end

	def <<(fileSpec)
		input = if fileSpec.respond_to?(:to_p4)
			fileSpec.to_p4
		else
			fileSpec
		end
		
		raise "Can't append absolute filespecs" if input[0..1] == '//'
		
		input = input[1..-1] until input[0..0] != '/'
		
		@fileSpec.chomp!('/')
		@fileSpec << '/' << fileSpec
	end
	
	def set(str)
		@fileSpec = str
	end
	
	def to_p4
		@fileSpec
	end
	
	def to_s
		"'#{@fileSpec}'"
	end
	
	def where
		Perforce.where(self)
	end
end