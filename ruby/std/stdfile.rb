require 'std/path'

module Stdfile
@@echo = 0
def Stdfile.echo=(i)
	@@echo = i
end

@@lame = false
def Stdfile.lame=(b)
	@@lame = b
end

@@stats = 0
def Stdfile.stats=(i)
	@@stats = i
end

# copy
# copies files from source to dest tree, creating dirs if necessary
# dst must not contain a filename
# newOnly - if true, doesn't copy files with the same modification times
# keepMTime - if true keeps the file modification times
def Stdfile.copy(src, dst, newOnly = false, keepMTime = true)
	src.chomp!('/')
	dst.chomp!('/')
	
	srcPath = src
	srcPath = File.dirname(srcPath) until FileTest.directory?(srcPath)

	if (FileTest.directory?(src)) then
		puts "#{src} is a directory, skipping..." if @@echo
		return
	elsif (FileTest.exists?(src)) then
		puts "#{src} -> #{dst}" if @@echo
		File.makedirs(File.dirname(dst)) if !@@lame
		File.copy(src, dst) if !@@lame
		return
	end
	
	dstPath = dst

	# stats
	totalBytes = 0
	totalFiles = 0

	files = Dir[src]

	startTime = Time.now

	files.each do |srcFile|
		dstFile = File.join(dstPath, srcFile.gsub(srcPath + '/',''))
		
		# skip directories
		next if File.directory?(srcFile)
		
		exists = File.exists?(dstFile)
		
		# newOnly check - skip if same modification time
		# fix -- ruby bug, sometimes out by an hour
		if (newOnly && exists && (File.mtime(srcFile).to_f - File.mtime(dstFile).to_f) <= 3600) then
			puts "#{srcFile} is same date as #{dstFile}" if @@echo > 1
			next
		end

		totalFiles = totalFiles + 1
		
		if @@stats then
			totalBytes += File.size(srcFile) if @@stats
		end
		
		# clear readonly permissions if dest file already exists
		File.chmod(0644, dstFile) if exists && !@@lame
		
		puts "#{srcFile} -> #{dstFile}" if @@echo
		File.makedirs(File.dirname(dstFile)) if !@@lame
		File.syscopy(srcFile, dstFile) if !@@lame
		
		if keepMTime && !@@lame then
			f = File.new(srcFile)
			oldmod = File.stat(dstFile).mode
			File.chmod(0644,dstFile)
			File.utime(f.atime, f.mtime, dstFile)
			File.chmod(oldmod, dstFile)
		end
	end

	endTime = Time.now

	if @@echo && newOnly && totalFiles == 0
		puts "\nAll files are up to date."
	end

	if @@stats then
		totalSecs = (endTime - startTime).to_f
		bytesPerSec = totalBytes / totalSecs;
		
		puts "\n#{totalFiles} files copied."
		puts "#{totalSecs.to_i} secs"
		puts "#{totalBytes} bytes"
		puts "#{bytesPerSec} bytes/sec"
	end
	
	return totalFiles
end

# compute differences between filespecs
# src = source Path. Specifies all files if no wildcard is given.
# dst = dest Path. Specifies all files if no wildcard is given.
# returns srcNotDst, dstNotSrc, common
def Stdfile.compareFilespecs(src, dst, calcModified = false)
	srcFiles = []
	dstFiles = []

	# get list of files in parallel
	# handy if one path is on a network path, not so important otherwise
	raise "compareFilespecs: No such source path '#{src}'." if !src.exists?
	raise "compareFilespecs: No such dest path '#{dst}'." if !dst.exists?
	
	srcFileThread = Thread.new {
		srcFiles = src.wild.ls
		#puts "Source file list generated."
		
		# remove base path from source files
		srcFiles.each do |s|
			s.relative!(src)
		end
	}
	dstFileThread = Thread.new {
		dstFiles = dst.wild.ls
		#puts "Destination file list generated."

		# remove base path from dest files
		dstFiles.each do |s|
			s.relative!(dst)
		end
	}
	
	srcFileThread.join
	dstFileThread.join
	
	srcNotDst = srcFiles - dstFiles
	dstNotSrc = dstFiles - srcFiles
	common = (srcFiles - srcNotDst) - dstNotSrc

	return srcNotDst, dstNotSrc, common
end

end
