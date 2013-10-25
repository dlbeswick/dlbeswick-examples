require 'delegate'
require 'std/platform_os'
require 'std/raise'
require 'std/tempfile_dlb'
require 'dl/import'

class FileMemoryMappedCommon
  include PlatformOSSpecific
  include Raise

  attr_accessor :path
    
  # Return a proc to be called on finalize.
  def self.finalizer_proc(*args)
    abstract
  end
  
  def initialize(path, mode='r+b', max_size=0)
    @mode = mode
    @max_size = max_size
    @path = path 
    @closed = true 

    open()    
  end

  def buffer_address
    @buffer.to_i
  end
  
  def close
    raise "Memory mapped file is already closed." if closed?
    xx "CLOSE 1 #{@path}"
    self.class.finalizer_proc(*finalizer_args()).call
    ObjectSpace.undefine_finalizer(self) if need_finalizer?
    @closed = true 
  end
  
  def reopen
    open()
  end
  
  def closed?
    @closed
  end

protected
  # Return an array of args to be passed to the finalizer
  def finalizer_args
    abstract
  end
  
  def need_finalizer?
    true
  end
  
  def open
    raise "Already open." if !closed?

#    raise "SetProcessWorkingSetSize failed (#{error(Kernel32.GetLastError)})" if Kernel32.SetProcessWorkingSetSize(Kernel32.GetCurrentProcess(), 1, 100 * 1024 * 1024) == 0
    
    xx 'c'
    if !@mode.include?('+')
      raise "Supplied mode was '#{@mode}', but mode must indicate both read and write permission." 
    end

    xx 'd'
    if @mode.include?('w') && (!@max_size || @max_size == 0)
      raise "You must specify a size if creating a new file (mode was '#{@mode}'.)" 
    end
    
    platform_specific_open()
    
    finalize_open()

    @closed = false 
  end
  
  def platform_specific_open
    abstract
  end
  
  def finalize_open
    ObjectSpace.define_finalizer(self, self.class.finalizer_proc(*finalizer_args())) if need_finalizer?
  end
end

if PlatformOS.linux?
class FileMemoryMappedLinux < FileMemoryMappedCommon
  include PlatformOSSpecific
  
  class ExceptionPosix < Exception
    def message
      @posix_error ||= Libc.posix_error_description
      super + " (Posix error: #{@posix_error})"
    end
  end
  
  def self.lazy_module_initialize_if_needed
    return if @module_initialized
      
    module_eval() do
      # create nested Module Libc
      const_set(:Libc, Module.new)
      
      Libc.module_eval do
        extend DL::Importer

        def self.libc_path 
          ldconfig_cache_lines = `ldconfig -p | grep libc\\\\.`.split($/)
          
          raise "ldconfig failed, can't find location of libc." if ldconfig_cache_lines.empty?
          
          result = /^.+? =\> (.+)/.match(ldconfig_cache_lines.first)
          
          raise "ldconfig output parse failed, can't find location of libc." if !result
          
          result[1]
        end
        
        dlload libc_path()
        
        extern 'int open(const char*, int)'
        extern 'int close(int)'
        extern 'char* strerror(int)'
        extern 'void *mmap(void*, long, int, int, int, long)'
        extern 'int munmap(void*, long)'
        
        const_set(:O_RDWR, 00000002)
        #O_CREAT = 00000100 tbd?
        
        const_set(:MAP_FAILED, -1)
        const_set(:PROT_READ, 0x1)
        const_set(:PROT_WRITE, 0x2)
        const_set(:MAP_SHARED, 0x001)

        @errno = import_symbol 'errno'
        
        def self.errno
          @errno[0]
        end
        
        def self.posix_error_description
          strerror(errno())
        end
      end
    end
    
    @module_initialized = true
  end

  def self.finalizer_proc(path, buffer, buffer_extent, file)
    proc do
      if Libc.close(file) != 0
        raise FileMemoryMappedLinux::ExceptionPosix.new("File close failed for file #{path}.")
      end
      
      if Libc.munmap(buffer, buffer_extent) != 0
        raise FileMemoryMappedLinux::ExceptionPosix.new("munmap failed for file #{path}.")
      end
    end
  end
  
  def initialize(path, mode='r+b', max_size=0)
    self.class.lazy_module_initialize_if_needed
    super
  end

  def close
    super
    @file = nil
    @buffer = nil
    @buffer_extent = 0
  end
  
protected
  # Return an array of args to be passed to the finalizer
  def finalizer_args
    [@path, @buffer, @buffer_extent, @file]
  end
  
  def platform_specific_open
    # note: O_CREAT tbd?
    result = Libc.open(@path.to_sys, Libc::O_RDWR)
    if result == -1
      raise ExceptionPosix.new("File open failed for file #{path}.")
    end
    
    @file = result
  
    max_size = if @max_size == 0
      @path.file_size
    else
      @max_size
    end
    
    @buffer_extent = max_size
    
    result = Libc.mmap(0, @buffer_extent, Libc::PROT_READ | Libc::PROT_WRITE, Libc::MAP_SHARED, @file, 0)
    if result == DL::CPtr.new(Libc::MAP_FAILED)
      raise ExceptionPosix.new("mmap failed for file #{@path}.")
    end
    
    @buffer = result
  end
end

if PlatformOS.windows?
  class FileMemoryMappedWindows < FileMemoryMappedCommon
    include PlatformOSSpecific
    #include Raise NOT WORKING!!!
  
    module Kernel32 
      extend DL::Importer
      dlload 'kernel32.dll'
      
      #HANDLE WINAPI CreateFile(
      #  __in      LPCTSTR lpFileName,
      #  __in      DWORD dwDesiredAccess,
      #  __in      DWORD dwShareMode,
      #  __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
      #  __in      DWORD dwCreationDisposition,
      #  __in      DWORD dwFlagsAndAttributes,
      #  __in_opt  HANDLE hTemplateFile
      #);
      extern 'int CreateFile(char*, int, int, void*, int, int, int)'
      
      #HANDLE WINAPI CreateFileMapping(
      #  __in      HANDLE hFile,
      #  __in_opt  LPSECURITY_ATTRIBUTES lpAttributes,
      #  __in      DWORD flProtect,
      #  __in      DWORD dwMaximumSizeHigh,
      #  __in      DWORD dwMaximumSizeLow,
      #  __in_opt  LPCTSTR lpName
      #);
      extern 'int CreateFileMapping(int, void*, int, int, int, char*)'
      
      #LPVOID WINAPI MapViewOfFile(
      #  __in  HANDLE hFileMappingObject,
      #  __in  DWORD dwDesiredAccess,
      #  __in  DWORD dwFileOffsetHigh,
      #  __in  DWORD dwFileOffsetLow,
      #  __in  SIZE_T dwNumberOfBytesToMap
      #);
      extern 'void* MapViewOfFile(int, int, int, int, int)'
      
      #DWORD WINAPI FormatMessage(
      #  __in      DWORD dwFlags,
      #  __in_opt  LPCVOID lpSource,
      #  __in      DWORD dwMessageId,
      #  __in      DWORD dwLanguageId,
      #  __out     LPTSTR lpBuffer,
      #  __in      DWORD nSize,
      #  __in_opt  va_list *Arguments
      #);
      extern 'int FormatMessage(int, void*, int, int, char**, int, void*)'
      
      extern 'int UnmapViewOfFile(void*)'
  
      extern 'int CloseHandle(int)'
      
      extern 'int GetLastError()'
  
      extern 'int GetCurrentProcess()'
  
      extern 'int SetProcessWorkingSetSize(int, int, int)'
    
      GENERIC_READ  = 0x80000000
      GENERIC_WRITE = 0x40000000
      
      CREATE_ALWAYS     = 2
      CREATE_NEW        = 1
      OPEN_ALWAYS       = 4
      OPEN_EXISTING     = 3
      TRUNCATE_EXISTING = 5
      
      FILE_MAP_WRITE      = 0x0002
      FILE_MAP_READ       = 0x0004
      
      FILE_ATTRIBUTE_NORMAL = 0x80
      
      PAGE_NOACCESS          = 0x01     
      PAGE_READONLY          = 0x02     
      PAGE_READWRITE         = 0x04     
      PAGE_WRITECOPY         = 0x08     
      PAGE_EXECUTE           = 0x10     
      PAGE_EXECUTE_READ      = 0x20     
      PAGE_EXECUTE_READWRITE = 0x40     
      PAGE_EXECUTE_WRITECOPY = 0x80     
      PAGE_GUARD             = 0x100     
      PAGE_NOCACHE           = 0x200     
      PAGE_WRITECOMBINE      = 0x400     
  
      FORMAT_MESSAGE_ALLOCATE_BUFFER  = 0x00000100
      FORMAT_MESSAGE_ARGUMENT_ARRAY   = 0x00002000
      FORMAT_MESSAGE_FROM_HMODULE     = 0x00000800
      FORMAT_MESSAGE_FROM_STRING      = 0x00000400
      FORMAT_MESSAGE_FROM_SYSTEM      = 0x00001000
      FORMAT_MESSAGE_IGNORE_INSERTS   = 0x00000200
    end
  
    attr_reader :path
      
    #exception_context :path
    
    def self.finalizer_proc(file, buffer, mapping, path)
      proc do
        xx "CLOSE #{path}"
        Kernel32.UnmapViewOfFile(buffer) if buffer
        Kernel32.CloseHandle(mapping) if mapping
        Kernel32.CloseHandle(file) if file
      end
    end
  
  protected
    def check_file(handle)
      if handle.to_i == -1
        error(Kernel32.GetLastError)
      end
    end
    
    def check_handle(handle)
      if handle.to_i == 0
        error(Kernel32.GetLastError)
      end
    end
    
    def error(code)
      output = '\0' * 2048
      
      size = Kernel32.FormatMessage(
        Kernel32::FORMAT_MESSAGE_FROM_SYSTEM,
        nil,
        code,
        0,
        output,
        output.length,
        nil
      )
      
      error_message = output.unpack("Z#{size}").to_s
      xx error_message
  
      raise "Error opening memory-mapped file."
    end
    
    def creation_disposition_from_mode(mode)
      if mode.include?('w+') || mode.include?('a')
        Kernel32::OPEN_ALWAYS
      elsif mode.include?('w')
        Kernel32::CREATE_ALWAYS
      else
        Kernel32::OPEN_EXISTING
      end
    end
  
    def file_access_from_mode(mode)
      if mode.include?('+')
        Kernel32::GENERIC_READ | Kernel32::GENERIC_WRITE 
      elsif mode.include?('w')
        Kernel32::GENERIC_WRITE
      elsif mode.include?('r')
        Kernel32::GENERIC_READ
      end
    end
    
    def platform_specific_open
      begin       
        @file = Kernel32.CreateFile(
          @path.to_sys, 
          file_access_from_mode(@mode), 
          0, 
          nil, 
          creation_disposition_from_mode(@mode), 
          Kernel32::FILE_ATTRIBUTE_NORMAL, 
          0
        )
        xx 'f'
        check_file(@file)
        
        xx 'g'
        # tbd: high byte from bignum
        @mapping = Kernel32.CreateFileMapping(@file, nil, page_protect_flags_from_mode(@mode), 0, @max_size, nil)
        check_handle(@mapping)
        
        xx 'h'
        xx @path.to_sys
        @buffer = Kernel32.MapViewOfFile(@mapping, view_access_from_mode(@mode), 0, 0, 0) 
        xx 'hhh'
        check_handle(@buffer)
        xx 'i'
  
      rescue
        xx 'Error opening memory-mapped file.'
        Kernel32.UnmapViewOfFile(@buffer) if @buffer
        Kernel32.CloseHandle(@mapping) if @mapping
        Kernel32.CloseHandle(@file) if @file
        raise
      end
    end
    
    def finalizer_args
      [@file, @buffer, @mapping, @path]
    end
    
    def page_protect_flags_from_mode(mode)
      if mode.include?('w') || mode.include?('+')
        Kernel32::PAGE_READWRITE
      else
        Kernel32::PAGE_READONLY
      end
    end
    
    def view_access_from_mode(mode)
      if mode.include?('+')
        Kernel32::FILE_MAP_WRITE | Kernel32::FILE_MAP_READ 
      elsif mode.include?('w')
        Kernel32::FILE_MAP_WRITE
      elsif mode.include?('r')
        Kernel32::FILE_MAP_READ
      end
    end
  end
end

class FileMemoryMappedNoFinalizer < FileMemoryMapped
protected
  def need_finalizer?
    false
  end
end

class FileMemoryMappedTemp < DLB::Tempfile
  def initialize(max_size, basename=nil, tmpdir=nil, extension=nil)
    @max_size = max_size
    super(basename, tmpdir, extension)
  end
  
  def buffer_address
    raise "No file opened." if !@tmpfile
    @tmpfile.buffer_address
  end
  
protected
  def _open_file(tmpname, mode, permission=0600)
    FileMemoryMappedNoFinalizer.new(Path.new(tmpname), 'w+', @max_size)
  end
end

require 'narray'

module NArrayMemoryMapped
  include Raise
  
  attr_accessor :file
  
  def self.byte(path, mode='r+b', *args)
    NArrayMemoryMapped.new(path, mode, NArray::BYTE, *args)
  end
  
  def self.sfloat(path, mode='r+b', *args)
    NArrayMemoryMapped.new(path, mode, NArray::SFLOAT, *args)
  end
  
  def self.make_narray(target_module, path, mode, *args_or_narray)
    if args_or_narray.length < 1
      raise 'Specify at least three arguments -- path, file mode, (NArray typecode,dims...)|(narray)'
    end
  
    source_narray = if args_or_narray[0].kind_of?(NArray)
      args_or_narray[0]
    else
      nil
    end
    
    typecode = if source_narray
      source_narray.typecode
    else
      args_or_narray[0]
    end
    
    bytes_per_element = case typecode
      when NArray::BYTE then 1
      when NArray::SFLOAT then 4
      else nil
    end
    
    if bytes_per_element == nil
      if typecode.kind_of?(Numeric)
        raise "Typecode '#{typecode}' not implemented."
      else
        raise "Supplied typecode was not a numeric value (#{typecode.class} given.)"
      end
    end
    
    if !source_narray && args_or_narray.length < 2
      if mode.include?('w')
        raise "You must specify a dimension for the NArray if the file is being created. Specify at least four arguments -- path, file mode, NArray typecode, dimension..."
      else
        raise "File path '#{path}' does not exist or was not given, and file mode '#{mode}' cannot create the file." if !path || !path.exists?
        args_or_narray << path.file_size / bytes_per_element
      end
    end
    
    total_size_dims = if source_narray
      source_narray.total
    else
      args_or_narray[1..-1].inject(1) { |total, dim| total *= dim }
    end

    total_size_bytes = total_size_dims * bytes_per_element
    
    @file = target_module._make_file(path, mode, total_size_bytes)
    
    xx 'naa'
    xx total_size_bytes
    narray = NArray.buffer_to_na(@file.buffer_address, total_size_bytes, *args_or_narray)
xx 'nab'
    narray.extend(target_module)
xx 'nac'
    narray.file = @file
xx 'nad'
    narray
  end

  def self.new(path, mode, *args_or_narray)
    NArrayMemoryMapped.make_narray(self, path, mode, *args_or_narray)
  end
  
  def close
    @file.close
  end
  
  def closed?
    @file.closed?
  end
  
  def reopen
    @file.reopen
  end
  
  def dup
    result = NArrayMemoryMappedTemp.new(typecode(), *sizes())
    result.add!(self)
    result
  end
  
  def path
    @file.path
  end
  
protected
  def self._make_file(path, mode, total_size_bytes)
    FileMemoryMapped.new(path, mode, total_size_bytes)
  end
end

module NArrayMemoryMappedTemp 
  include NArrayMemoryMapped
  
  def self.byte(*args)
    NArrayMemoryMappedTemp.new(NArray::BYTE, *args)
  end
  
  def self.sfloat(*args)
    NArrayMemoryMappedTemp.new(NArray::SFLOAT, *args)
  end
  
  def self.new(*args_or_narray)
    NArrayMemoryMapped.make_narray(self, nil, 'w+', *args_or_narray)
  end
  
  def keep!
    @file.keep!
  end
  
protected
  def self._make_file(path, mode, total_size_bytes)
    FileMemoryMappedTemp.new(total_size_bytes)
  end
end
elsif PlatformOS.linux?
  class FileMemoryMappedLinux
    include PlatformOSSpecific
    
    def initialize(io, mode='rb')
      Platform.specific
    end
  end
end

def test
  require 'narray'  
  map = FileMemoryMapped.new(Path.new('test.txt'), 'w+b', 1024)
  ary = NArray.buffer_to_na(map.buffer_address, 1024, NArray::BYTE)
  i = -1
  ary.collect! do |e|
    i += 1 
    'A'[0] + (i % 64)
  end
  map.close
  exit
end
