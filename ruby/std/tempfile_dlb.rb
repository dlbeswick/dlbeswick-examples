#
# tempfile - manipulates temporary files
#
# $Id: tempfile.rb 11708 2007-02-12 23:01:19Z shyouhei $
# dbeswick: added 'binary' option
#

require 'std/abstract'
require 'delegate'
require 'tmpdir'
require 'std/path'

module DLB
  # A class for managing temporary files.  This library is written to be
  # thread safe.
  class TempfileBase < DelegateClass(File)
    include Abstract
    
    MAX_TRY = 10
    @@cleanlist = []
  
    # dbeswick: create a lock and choose a name, but don't open the file.
    def self.ready(basename=nil, tmpdir=nil, extension=nil, binary=nil)
      result = allocate()
      result._ready!(basename, tmpdir, extension, binary)
      
      result
    end
  
    def _ready!(basename, tmpdir, extension, binary)
      basename=Path.new($0).file.sub_invalid.literal if !basename
      tmpdir='./tmp' if !tmpdir
      extension='tmp' if !extension
      binary=true if binary.nil?
      
      Dir.mkdir(tmpdir) if !File.exist?(tmpdir)
      
      @binary = binary
      
      if $SAFE > 0 and tmpdir.tainted?
        tmpdir = '/tmp'
      end
  
      @lock = nil
      n = failure = 0
      
      begin
        Thread.critical = true if PlatformRuby.v18? # hack, fix with v19 impl
  
        begin
          tmpname = File.join(tmpdir, make_tmpname(basename, n, extension))
          @lock = tmpname + '.lock'
          n += 1
        end while @@cleanlist.include?(tmpname) or
        File.exist?(@lock) or File.exist?(tmpname)
        Dir.mkdir(@lock)
      rescue
        failure += 1
        retry if failure < MAX_TRY
        raise "cannot generate tempfile `%s'" % tmpname
      ensure
        Thread.critical = false if PlatformRuby.v18? # hack, fix with v19 impl
      end
  
      @data = [tmpname]
      @clean_proc = DLB::Tempfile.callback(@data)
      ObjectSpace.define_finalizer(self, @clean_proc)
  
      @mode = File::RDWR|File::CREAT|File::EXCL
      
      # dbeswick
      begin
        @mode |= File::BINARY if binary
      rescue NameError
      end
      
      @tmpname = tmpname
      @@cleanlist << @tmpname
      @data[1] = nil
      @data[2] = @@cleanlist
      @data[3] = @lock
    end
    
    # Creates a temporary file of mode 0600 in the temporary directory
    # whose name is basename.pid.n and opens with mode "w+".  A Tempfile
    # object works just like a File object.
    #
    # If tmpdir is omitted, the temporary directory is determined by
    # Dir::tmpdir provided by 'tmpdir.rb'.
    # When $SAFE > 0 and the given tmpdir is tainted, it uses
    # /tmp. (Note that ENV values are tainted by default)
    def initialize(basename=nil, tmpdir=nil, extension=nil, binary=nil)
      _ready!(basename, tmpdir, extension, binary)
      
      @tmpfile = _open_file(@tmpname, @mode, 0600)
      @data[1] = @tmpfile
      @data[2] = @@cleanlist
      
      super(@tmpfile)
  
      # Now we have all the File/IO methods defined, you must not
      # carelessly put bare puts(), etc. after this.
  
      Dir.rmdir(@lock)
    end
    
    def keep!
      ObjectSpace.undefine_finalizer(self)
    end
  
    # dbeswick:
    def make_tmpname(basename, n, extension)
      if !extension.empty?
        sprintf('%s.%d.%d.%s', basename, $$, n, extension)
      else
        sprintf('%s.%d.%d', basename, $$, n)
      end
    end
    private :make_tmpname
  
    # Opens or reopens the file with mode "r+".
    def open
      @tmpfile.close if @tmpfile
      
      # dbeswick
      mode = if !@binary
        'r+'
      else
        'r+b'
      end
      
      @tmpfile = _open_file(@tmpname, mode)
      @data[1] = @tmpfile
      __setobj__(@tmpfile)
    end
  
    def _close  # :nodoc:
      @tmpfile.close if @tmpfile
      Dir.rmdir(@lock) if File.exist?(@lock)
      @data[3] = @data[1] = @tmpfile = nil
    end    
    protected :_close
  
    # Closes the file.  If the optional flag is true, unlinks the file
    # after closing.
    #
    # If you don't explicitly unlink the temporary file, the removal
    # will be delayed until the object is finalized.
    def close(unlink_now=false)
      if unlink_now
        close!
      else
        _close
      end
    end
  
    # Closes and unlinks the file.
    def close!
      _close
      @clean_proc.call
      ObjectSpace.undefine_finalizer(self)
    end
  
    # Unlinks the file.  On UNIX-like systems, it is often a good idea
    # to unlink a temporary file immediately after creating and opening
    # it, because it leaves other programs zero chance to access the
    # file.
    def unlink
      # keep this order for thread safeness
      begin
        File.unlink(@tmpname) if File.exist?(@tmpname)
        @@cleanlist.delete(@tmpname)
        @data = @tmpname = nil
        ObjectSpace.undefine_finalizer(self)
      rescue Errno::EACCES
        # may not be able to unlink on Windows; just ignore
      end
    end
    alias delete unlink
  
    # Returns the full path name of the temporary file.
    def path
      Path.new(@tmpname)
    end
  
    # Returns the size of the temporary file.  As a side effect, the IO
    # buffer is flushed before determining the size.
    def size
      if @tmpfile
        @tmpfile.flush
        @tmpfile.stat.size
      else
        0
      end
    end
    alias length size
  
    class << self
      def callback(data)  # :nodoc:
        xx "CLEANUP TMP FILE"
        xx data
        pid = $$
        lambda{
          if pid == $$
            path, tmpfile, cleanlist, lock = *data
            
            print "removing ", path, "..." if $DEBUG
            
            tmpfile.close if tmpfile
            
            # keep this order for thread safeness
            File.unlink(path) if File.exist?(path)
            Dir.rmdir(lock) if !lock.nil? && File.exist?(lock)
            cleanlist.delete(path) if cleanlist
            
            print "done\n" if $DEBUG
          end
          }
        end
  
      # If no block is given, this is a synonym for new().
      #
      # If a block is given, it will be passed tempfile as an argument,
      # and the tempfile will automatically be closed when the block
      # terminates.  In this case, open() returns nil.
      def open(*args)
        tempfile = new(*args)
  
        if block_given?
          begin
            yield(tempfile)
          ensure
            tempfile.close
          end
          
          nil
        else
          tempfile
        end
      end
    end
  
  protected
    def _open_file(tmpname, mode, permission)
      abstract
    end
  end
  
  class Tempfile < DLB::TempfileBase
    def initialize(basename=nil, tmpdir=nil, extension=nil, binary=nil)
      super
    end
  
  protected
    def _open_file(tmpname, mode, permission=0600)
      File.open(tmpname, mode, permission)
    end
  end
end

if __FILE__ == $0
#  $DEBUG = true
  f = DLB::Tempfile.new("foo")
  f.print("foo\n")
  f.close
  f.open
  p f.gets # => "foo\n"
  f.close!
end
