# Path
# A path is any filesystem path and may include Dir-style wildcards.
# Nearly all functions accept Path objects rather than strings.

require 'std/deprecatable'
require 'fileutils'
require 'std/module_logging'
require 'std/platform'
require 'std/raise'
require 'std/SafeThread'
require 'pathname'
require 'uri'

class PathCommon
  include Comparable
  include Deprecatable
  include ModuleLogging
  include Raise

  exception_context { to_s() }
  
  @@wildChars = '*'

  def self.allow_trailing_period?
    PlatformOS.specific
  end
  
  # true if string or path contains wildcard characters
  def self.wild?(string_or_path)
    if string_or_path.respond_to?(:wild?)
      string_or_path.wild?
    else
      (string_or_path =~ /[#{Regexp.escape(@@wildChars)}]/) != nil
    end
  end

  def initialize(path_or_string_or_array = nil, is_directory = nil)
    @segments = []
    
    case path_or_string_or_array
    when Array
      @segments = path_or_string_or_array.dup
    when String
      self.path=(self.class.split_string(path_or_string_or_array))
    when Path
      self.path=(path_or_string_or_array)
    when NilClass
      # do nothing
    else
      raise "Input must be String, Array or Path object (was #{path_or_string_or_array.class})."
    end
    
    if !is_directory.nil?
      @is_directory = is_directory
    end
  end
  # initialize should be made private in the near future
  #private :initialize
  
  # Create a new path object and specify that it points to a directory. 
  def self.to_directory(path_or_string_or_array)
    new(path_or_string_or_array, true)
  end

  # As for to_directory.
  def self.to_dir(path_or_string_or_array)
    new(path_or_string_or_array, true)
  end
  
  # Create a new path object and specify that it points to a file. 
  def self.to_file(path_or_string_or_array)
    new(path_or_string_or_array, false)
  end

  # Home directory.
  def self.home
    new(Dir.home)
  end
  
  def initialize_copy(from_obj)
    self.path=(from_obj)
    self
  end

  def ==(rhs)
    if rhs.kind_of?(PathCommon)
      @segments == rhs.segments
    else
      false
    end
  end
  
  def <=>(rhs)
    # optimisation -- prevent exception if compared with a non-Path object.
    # Exception handling was showing up during profiling. 
    if rhs.kind_of?(PathCommon)  
      literal() <=> rhs.literal
    else
      -1
    end
  end  

  # returns a path formed by splitting the input string into segments where a slash, backslash, or the
  # platform's path separator is found.
  def self.split_string(str)
    return [] if str.nil?
    return [] if str.empty?
    
    segments = str.split(separator_regexp())
    
    is_unc = segments.length > 2 && segments[0].empty? && segments[1].empty?
    if is_unc
      segments = ['/'] + segments[2..-1]
    end
    
    # remove duplicate path separators
    i = 0
    segments.delete_if do |segment|
      delete = i > 0 && segment.empty?
      i += 1
      delete
    end
    
    result = Path.new(segments)
    result.instance_variable_set(:@is_directory, true) if (str[-1..-1] =~ separator_regexp) == 0
    result
  end
  
  # return the path appended with a path or path segment. input is a path object or a string.
  # see notes for operator '<<'.
  def +(other)
    ret = self.clone
    ret << other.dup
    ret
  end

  # append a path or path segment. input is a path object or a string.
  # the result of adding string input will always be considered a path to a directory.
  # if you wish for the path to refer to a file, construct a Path object or use 'with_file'. 
  # note that for string input, included path separators are considered part of a single segment.
  # the path separator characters will finally be converted to 'invalid characters' on a call to 'to_sys'.
  # to append string input split into segments, create a new Path object from the string and append the result.
  def <<(other)
    if other.kind_of?(Path)
      if other.has_root? && !empty?
        raise "Cannot append a path that has a root (#{self} + #{other}}."
      end
      
      @segments += other.segments
      @is_directory = other.directory?
      
      return self
    end
    
    return if !other || other.empty?
    
    if other[0..0] == self.class.platformSeparator()
      raise "Concatenating paths that begin with platform separators creates ambiguity on some platforms, and so is not allowed. (while concatenating '#{other}')"
    end

    # string input always results in a directory reference.
    @is_directory = true
    
    @segments << other
    
    self
  end

  # returns the absolute representation of the path
  def absolute(root=nil)
    ret = self.clone
    ret.absolute!(root)
    return ret
  end

  def absolute!(root=nil)
    return self if absolute?
    
    if !root
      self.path = Path.new(File.expand_path(to_sys()))
    else
      self.path = root.unwild + self
    end
  end

  # true if the path is absolute (starts with x:/ or / and doesn't contain '..')
  def absolute?
    return false if empty?
    has_root? && @segments.find { |x| x == '..' } == nil
  end
  
  def chmod(mode)
    File.chmod(mode, to_sys())
  end

  def copy(dest_path)
    sources = self.ls
    
    raise "Attempt to copy multiple files to a single file." if !dest_path.directory? && sources.length > 1
    raise "No source file exists." if sources.empty?
    
    sources.each do |f|
      log { "#{f} -> #{dest_path}" }
      FileUtils.cp(f.to_sys, dest_path.to_sys)
    end
  end
  
  def copy_to(destPath)
    deprecated_since(2009,07,:copy)
  end

  def delete(force=false)
    return if !exists?

    File.chmod(0o666, to_sys) if force

    if directory? then
      Dir.delete(to_sys)
    else
      File.delete(to_sys)
    end
  end
  
  def deleteTree(force=false)
    return if !exists? || !directory?
    
    block = Proc.new { |dirPath|
      dir_glob[dirPath + "**"].each do |f|
        p = Path.new(f).absolute
        if p.directory?
          block.call(p)
        end

        p.delete(force)
      end
    }
    
    block.call(self)

    delete(force)
  end

  # compute differences between paths.
  # returns srcNotDst, dstNotSrc, common
  def diff(dst)
    raise "Source and target must be directories or have wildcards" if file? || dst.file?
    
    srcFiles = []
    dstFiles = []
    
    # get list of files in parallel
    # handy if one path is on a network path, not so important otherwise
    raise "No such source path '#{self.unwild}'." if !self.unwild.exists?
    raise "No such dest path '#{dst.unwild}'." if !dst.unwild.exists?
    
    srcFileThread = SafeThread.new {
      srcFiles = self.wild.ls
      
      # remove base path from source files
      srcFiles.each do |s|
        s.relative!(self)
      end
    }
    dstFileThread = SafeThread.new {
      dstFiles = dst.wild.ls
      
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

  def dir
    deprecated_since(2009,10,:no_file)
  end
  
  # return true if the path is designated as a directory, or if its target is an existing directory.
  def directory?
    # tests will be a little less exact when operating tainted.
    # tbd: is the explicit file test still required when the is_directory/trailing slash convention exists?
    if $SAFE == 0
      @is_directory == true || (@is_directory.nil? && FileTest.directory?(self.to_sys))
    else
      @is_directory == true
    end
  end
  
  # Uses the Dir class to enumerate files in a directory. Path must refer to a directory.
  def each_path(recurse=true)
    raise "Path does not point to a directory (#{self})" if !directory?
    
    currentPath = self.absolute
    
    dir = [currentPath.to_dir()]
    
    while !dir.empty? do
      entry = dir.last.read
      
      if entry == nil
        if !dir.empty?
          currentPath += Path.new("..")
          currentPath.absolute!
          dir.pop.close
        else
          break
        end
      elsif entry == "." || entry == ".."
        # ignore ".." and "."
        next
      else
        newPath = currentPath + Path.new(entry)        

        # recurse through directories by adding a directory entry to the stack
        if newPath.directory? && recurse
          currentPath = newPath
          
          # catch attempts to open bad directory entries
          begin
            dir << newPath.to_dir
            entry = nil
          rescue
            next
          end
        else
          yield newPath
        end
      end
    end
  end
  
  def empty?
    @segments.empty?
  end
  
  # must implement this to support set difference
  def eql?(rhs)
    rhs.kind_of?(self.class) && (self <=> rhs) == 0
  end

  def exists?
    if wild?
      raise 'Cannot check for existance of wildcard spec.'
    else
      File.exists?(to_sys)
    end
  end
  
  # returns a string, excludes extension designator
  def extension
    return '' if !file()
    r = /.+\.(.*)/.match(@segments.last)
    return '' if !r
    return r[1]
  end

  # accepts a string. sets the path's extension.
  def extension=(str)
    return if str.empty?
    raise 'Omit leading periods from the extension.' if str[0..0] == '.'
    no_ext_path = no_extension()
    segments = no_ext_path.instance_variable_get(:@segments)
    segments.last << '.' << str
    self.path = no_ext_path
  end
  
  # returns true if the extension is the same as the path given.
  # takes the case sensitivity of the filesystem into account
  def extension_eql?(path)
    if Path.platformCaseSensitive?
      extension().downcase == path.extension.downcase
    else
      extension() == path.extension
    end
  end

  def self.extension_eql?(extension, path)
    if Path.platformCaseSensitive?
      extension == path.extension.downcase
    else
      extension == path.extension
    end
  end
  
  # returns the file portion of the path as a path object, including file extension.
  # returns an empty path if the path doesn't point to a file.
  # note: it cannot always be known if a path is not a file unless the file is present, or the path has
  # been explicitly designated as a directory.
  # this is because both directories and files can have an extension of nothing or "." in posix.
  def file
    if !file?
      Path.new
    else
      Path.new(@segments.last)
    end
  end

  # Replace the file part of a path with the given string. All other path information is preserved.
  def file=(str)
    # Given file="hello2.txt", ensure that paths in the format 'hello.txt' result in 'hello2.txt', and
    # paths in the format './hello.txt' result in './hello2.txt'.
    #
    # If the path is just a filename without any relative or absolute path, then replace the path with the
    # given string in its entirety.
    if file? && segments().length == 1
      self.path = Path.to_file(str)
    else
      self.path = self.no_file.segments + [str]
      @is_directory = false
    end
  end
  
  def file?
    !wild? && !directory?
  end

  def file_size
    File.size(to_sys())
  end
  
  def fileSize
    deprecated_since(2009, 07, :file_size)
  end
  
  # converts to utf-8 from the internal ruby representation.
  # the internal representation depends on the host filesystem and ruby implementation.
  # see 'localise' for the reverse operation.
  def internationalise!
    @segments.collect! do |segment|
      segment.encode('utf-8', PlatformOS.filenameEncoding)
    end
  end

  def internationalise
    newPath = self.clone
    newPath.internationalise!
    newPath
  end
  
  # must implement this to support set difference
  def hash
    @segments.hash
  end
  
  def has_root?
    abstract
  end
  
  def last
    return Path.new(@segments.last)
  end
  
  # if true, then no transformation is performed before return the OS representation
  def literal?
    @literal != nil && @literal == true
  end

  def literal=(b)
    @literal=b
  end

  # returns a string representation the path using forward slashes as separators. 
  def literal
    to_sys('/')
  end
  
  # converts to the internal ruby representation from utf-8.
  # the internal representation depends on the host filesystem and ruby implementation.
  # see 'internationalise' for the reverse operation.
  def localise!(src_encoding = nil)
    @segments.collect! do |segment|
      encoding = if src_encoding
                   src_encoding
                 else
                   segment.encoding.name
                 end
      
      segment.encode(PlatformOS.filenameEncoding, encoding)
    end
  end

  def localise(src_encoding = nil)
    newPath = self.clone
    newPath.localise!(src_encoding)
    newPath
  end
  
  # Get all Paths matching the fileSpec denoted by this Path.
  # Returns an array containing the path if the path points to a single file.
  # Returns the contents of the directory if the path points to a directory.
  # Passes each result to the block if given, otherwise returns an array.
  def ls(append_spec=nil, &block)
    subject = if append_spec
                self + append_spec
              else
                self
              end
    
    result = if subject.file?
               [self]
             else
               dir_path = if subject.wild?
                            subject
                          else
                            subject.with_file('*.*')
                          end
               
               if !block
                 array = []
                 
                 dir_glob(dir_path).each do |f|
          array << Path.new(f)
        end
                 
                 array
               else
                 dir_glob(dir_path).each do |f|
          yield Path.new(f)
        end
               end
             end
  end

  def makeDirs
    deprecated_since(2009,07,:make_dirs)
  end
  
  def make_dirs
    # tbd: test for directory. see directory tbd (no extension file case).
    FileUtils.mkdir_p(no_file().to_sys)
  end
  
  def ctime
    File.ctime(to_sys())
  end
  
  def mtime
    File.mtime(to_sys())
  end
  
  # returns true if given path is newer than this path
  def newer_path?(path)
    !File.exists?(path.to_sys) || (File.exists?(to_sys()) && (File.mtime(to_sys).to_f - File.mtime(path.to_sys).to_f) > 3600)
  end
  
  # returns true if path's modification time is greater than given modification time
  def newer_time?(mtime)
    File.exists?(to_sys) && (File.mtime(to_sys()).to_f - mtime.to_f) > 3600
  end
  
  def no_extension
    return dup() if !file?
    return dup() if extension().empty?
    result = dup()
    result.file = file().literal()[0...-(extension().length + 1)]
    result
  end

  # strips the file from a path.
  # returns the existing path if it doesn't point to a file.
  # return '.' if the path consists only of a filename.
  #
  # The returned path always refers to a directory, so note that this command:
  #  Path.new('file.ext').no_file.with_file('file2.ext') => './file2.ext'
  # whereas this command:
  #  Path.new('file.ext').file='file2.ext' => 'file2.ext'
  #
  # Therefore, always prefer 'file=' when changing filenames, so that path information is preserved.
  def no_file
    return clone() if directory?
    
    if @segments.length == 1
      Path.to_directory('.')
    else
      Path.to_directory(@segments[0...-1])
    end
  end
  alias :directory :no_file
  
  # mimics File.open method
  def open(flags=nil, &block)
    File.open(to_sys(),flags) do |file|
      yield file
    end
  end

  # returns the parent of the path.
  # returns the directory of a file path or filespec.
  # returns the parent of a directory.
  # returns nil if it's the root or empty. 
  def parent
    if empty?
      nil
    elsif wild? || file?
      no_file()
    elsif root?
      nil
    else
      path = absolute()
      Path.new(@segments[0...-1], true)
    end
  end

  # returns the first directory in the hierarchy with a name matching the one given.
  def parent_with_name(name, inclusive = true)
    if inclusive
      Algorithm.recurse_from_object(self, :parent) do |path|
        return path if matcher(name).match(path._segments.last)
      end
    else
      Algorithm.recurse(self, :parent) do |path|
        return path if matcher(name).match(path._segments.last)
      end
    end
  end
  
  # sets path
  def path=(s)
    if s.kind_of?(Path)
      @segments = s.instance_variable_get(:@segments).dup
      @is_directory = s.instance_variable_get(:@is_directory)
    elsif (s.kind_of?(Array))
      @segments = s.dup
    else
      raise "Only Path and Array objects are valid input (input was #{s.class})."
    end
  end

  def self.platformCaseSensitive?
    Platform.specific
  end
  
  def self.platformSeparator
    Platform.specific
  end
  
  # Re-interprets a path such that it becomes a child of the given root path.
  # For absolute paths, the leading slash or drive specification is removed and the resulting path
  # placed below the given root.
  def rebase(root)
    if absolute?
      root + Path.new(@segments[1..-1])
    else
      root + self
    end
  end
  
  def rebase!(root)
    self.path = rebase(root)
  end

  # Returns a path with all path segments removed up until the segment matching the given regexp.
  # Returns nil if no match occurs.
  def rebase_from_segment(regexp)
    new_segments = @segments.dup
    while !new_segments.empty? && (new_segments.first =~ regexp) == nil
      new_segments.shift
    end

    if new_segments.empty?
      nil
    else
      Path.new(new_segments, @is_directory)
    end
  end

  def realpath
    Path.new(Pathname.new(to_sys()).realpath.to_s, @is_directory)
  end
  
  # converts to a path relative to the given path
  # i.e. (/path/to/a/file.txt).relative(/path/to) == a/file.txt
  def relative!(path)
    unwilded = path.unwild
    
    new_path = if Path.platformCaseSensitive?
                 raise "Relative transform failed: Path #{self} has no common base with path #{unwilded}" if !(literal() =~ /#{Regexp.escape(unwilded.literal)}/)
                 literal().gsub(/#{Regexp.escape(unwilded.literal)}\/?/, '')
               else
                 raise "Relative transform failed: Path #{self} has no common base with path #{unwilded}" if !(literal() =~ /#{Regexp.escape(unwilded.literal)}/i)
                 literal().gsub(/#{Regexp.escape(unwilded.literal)}\/?/i, '')
               end
    
    if new_path.empty?
      if file?
        new_path = self
      else
        new_path = '.'
      end
    end

    self.path = Path.new(new_path, directory?)
  end

  def relative(path)
    ret = self.clone
    ret.relative!(path)
    return ret
  end
  
  def root?
    @segments.length == 1 && absolute? 
  end

  def segments
    @segments.dup
  end

  def stat
    File::Stat.new(to_sys())
  end
  
  def subExtension(e)
    deprecated_since(2009, 07, :with_extension, e)
  end

  # substitutes the given character for any characters in the path that are invalid in operating system representation 
  def sub_invalid(replacement_str = '_')
    ret = self.clone
    ret.sub_invalid!(replacement_str)
    ret
  end

  def sub_invalid!(replacement_str = '_')
    result = []
    
    is_directory = directory?
    is_file = file?
    
    @segments.each_with_index do |segment, i|
      is_last = i == @segments.length - 1
      result << self.class.sub_invalid(is_directory || (file? && !is_last), segment, replacement_str)
    end
    
    @segments = result
  end
  
  def self.sub_invalid(is_directory_segment, input_string, replacement_str = '_')
    result = input_string.gsub(/#{invalidOSCharsRegExpCharClass}/, replacement_str)
    
    if is_directory_segment && !allow_trailing_period?
      match = /([^.].*)\.+$/.match(result)
      if match
        result = match[1]
      end
    end
    
    result
  end
  
  def subInvalid(*args)
    deprecated_since(2009, 07, :sub_invalid, *args)
  end

  def subInvalid!(*args)
    deprecated_since(2009, 07, :sub_invalid!, *args)
  end
  
  # returns the path target in a format suitable for use in the system's command shell 
  def to_cmdline
    PlatformOS.shell_cmdline_format_path(to_sys())
  end
  
  # returns a File
  def to_file(flags=nil, &block)
    raise "Use 'open' with a block." if block
    File.new(to_sys(),flags)
  end
  
  # returns a Dir instance
  def to_dir
    Dir.open(to_sys())
  end
  
  def to_s
    if !empty?
      "'#{literal()}'"
    else
      "<empty path>"
    end
  end
  
  # return path with operating system separator convention and string encoding 
  def to_sys(separator = nil)
    if !separator
      separator = if @literal
                    '/'
                  else
                    self.class.platformSeparator
                  end
    end
    
    result = @segments.join(separator)
    result << separator if @is_directory
    result
  end

  # Returns a new path where each segment is URI escaped.
  def uri_encoded
    Path.new(segments.collect{ |segment| URI.encode(segment) }, @is_directory)
  end

  # removes wildcard parts
  def unwild!
    until !wild?
      self.path = parent()
    end
  end
  
  def unwild
    ret = self.clone
    ret.unwild!		
    return ret
  end
  
  def utime(access, modified)
    File.utime(access, modified, to_sys())
  end

  # true if path contains wildcard characters	
  def wild?
    Path.wild?(literal())
  end
  
  # converts to wild specification by adding wildcard denoting 'all files and folders,' if no wildcard specification is present already
  def wild
    ret = self.clone
    ret.wild!
    return ret
  end	
  
  def wild!
    raise "Can't 'wild' a path that points to a file." if file?
    
    if !wild?
      self << '**' << '*.*'
    end
  end	

  # returns wildcard parts
  def wildPart
    return /.*?([#{@@wildChars}].*)/.match(literal())[1]
  end
  
  # accepts a string. returns a copy of the path with the given extension.
  def with_file(str)
    # This condition was instituted to avoid errors. Paths without extensions or trailing
    # slashes can be files or directories. It's ambiguous and can't be known when the file
    # doesn't exist. Calling with_file on a path mistakenly constructed with a trailing file
    # part will produce an incorrect result, as the trailing directory will be discarded.
    # So with_file should be favoured unless it's known for sure that the path is constructed
    # unambiguously, and then with_file_replace can be used. 
    raise "Path already has a filename. If this is not an error, please call 'with_file_replace'." if !directory?
    
    raise "Argument must be string." if !str.kind_of?(String)
    raise "Path separators are invalid in filenames (input: '#{str}')." if self.class.has_separator?(str) 
    result = dup()
    result.file = str
    result
  end
  
  def with_file_replace(str)
    no_file().with_file(str)
  end

  # accepts a string. returns a copy of the path with the given extension.
  def with_extension(str)
    raise "Argument must be string." if !str.kind_of?(String) 
    result = dup()
    result.extension = str
    result
  end
  
protected
  def _segments
    @segments
  end

  # performs a Dir[] glob while taking platform considerations into account. This should
  # be used instead of a straight Dir[].
  def dir_glob(glob_path=self,&block)
    glob_str = glob_path.literal
    
    escape_chars = self.class.dir_glob_escape_chars()
    glob_str.gsub!(/[#{Regexp.escape(escape_chars)}]/) { |m| "\\#{m}" }
    
    Dir[glob_str,&block]
  end

  def matcher(literal_or_regexp)
    if literal_or_regexp.kind_of?(String)
      Regexp.new(Regexp.quote(literal_or_regexp), self.class.regexp_match_options)
    else
      Regexp.new(literal_or_regexp, self.class.regexp_match_options)
    end
  end

  def self.regexp_match_options
    if platformCaseSensitive?
      Regexp::IGNORECASE
    else
      0
    end
  end

  # returns the characters that are invalid in operating system representations
  def self.invalidOSChars
    @invalid_os_chars ||= '\\/:*?"<>|' + (0..31).to_a.collect{ |x| x.chr }.join
  end
  
  def self.invalidOSCharsRegExpCharClass
    escaped = Array(invalidOSChars()).collect { |x| "\\#{x}" }
    "[#{escaped.join}]"
  end

  def self.separator_regexp 
    /[\/\\#{Regexp.escape(platformSeparator())}]/
  end
  
  def self.has_separator?(string)
    separator_regexp.match(string) != nil
  end

  # characters that have to be escaped for path components for dir globbing
  def self.dir_glob_escape_chars
    "[]"
  end
end

# windows
class PathWindows < PathCommon
  def self.allow_trailing_period?
    false
  end
  
  def self.platformCaseSensitive?
    false
  end

  def has_root?
    /\:/ =~ @segments.first || @segments.first == '/'
  end
  
  def self.platformSeparator
    '\\'
  end
end

# unix-like
module PathUnixLike
  include IncludableClassMethods
  
  module ClassMethods
    def allow_trailing_period?
      # Note: this is really more of an example of a filesystem issue rather than an operating system
      # issue. Or is it? NTFS actually supports trailing periods, but the Windows tools don't handle it
      # properly. Anyway, the real issue is that when running on linux, it's not helpful to put trailing
      # periods on the names of files that are being saved to NTFS volumes. The correct solution will lazily
      # extract the filesystem type from paths and cache the result for later use as a class variable mapping
      # from drive letters/identifiers to filesystem types. I don't have time to do that so it's false
      # for now.
      # Look into the sys-filesystem gem.
      #true
      false
    end
    
    def platformSeparator
      '/'
    end
  end

  def has_root?
    !@segments.empty? && @segments.first.empty?
  end
end

# osx
class PathOSX < PathCommon
  include PathUnixLike
  
  def self.platformCaseSensitive?
    false
  end
end

class PathLinux < PathCommon
  include PathUnixLike
  
  def self.platformCaseSensitive?
    true
  end
end

require 'std/platform_os' if !Object.const_defined?(:PlatformOS)
PlatformOS.setup(:Path)
