require 'drb'
require 'std/abstract'
require 'std/view/Viewable'

$drb_debug_incoming = false

# Prints the caller of the rpc method if send_request fails. This can be useful for debugging
# recycled object id errors.
$drb_debug_caller_info_in_send_request_exception = false

# Prints every incoming drb call. Useful for debugging hangs.
$drb_debug_recv_request_trace = false && $drb_debug_caller_info_in_send_request_exception

# allows dumping of arrays containing drb objects. if not used, a marshalling error occurs and any array
# containing an undumped object is sent as an undumped object itself, potentially increasing the 
# number of rpcs.
$drb_fix_undumped_array = true

# adds a mixin allowing objects to define dump and marshal operations seperately.
$drb_specific_dump = true

# Provides a method that always queries the Object itself and never calls a remote server to do so. 
$drb_no_drb_respond_to = true
$drb_idconv_debug = true

$drb_instance_variable_get_distribute = true
$drb_comparable_distribute = true
$drb_singlethreaded_blocks = true

# Adds method "ensure_authoritive" to Object, that attempts to make undumped all objects relevant 
# to the target, so that changes will persist on the server.
$drb_ensure_authoritive = true

# When this isn't active, exceptions encountered while handling rpc requests can sometimes fail
# silently and cause the other system to crash with a "DRbConnError" after the server terminates as
# a consequence.
$drb_improved_exception_handling_on_perform = true

$drb_extra_debugging = true
$drb_extra_safety = true

# Adds a method DRb.instance_here? that accepts object instance references and DRbObjects.
$drb_instance_here = true

# DRb.here? can return false when comparing two druby addresses that are equivalent, such as 'localhost'
# and '127.0.0.1'. IPSocket.getaddress is used to disambiguate where necessary. Could this be a
# performance issue when many reverse-dns lookups are required, and could this be better solved using
# a UUID for each server and storing that UUID in every DRbObject?
$drb_fix_here_equivalence_with_getaddress = true

# Allows method calls to return DRbObject instances on a selective basis, without necessitating the
# inclusion of the DRbUndumped module.
$drb_method_missing_undumped_result_option = true

# Adds a module allowing classes to ensure that they are always dumped over drb connections.
$drb_module_never_undumped = true

# Adds method stubs to the DRb module defining an interface to make authoritive method calls.
# Authoritive calls are those that always happen on the server.
$drb_authoritive_send = true

# Adds method "authoritive_new" to Object. The call is always delegated to 'new' on the server, 
# and the resulting object is always DRbUndumped. 
$drb_authoritive_new = true && $drb_authoritive_send
 
# Make drb objects comparable across servers, and include a drb_equal? equality test that works
# similarly to Object.equal?
$drb_object_comparable = true

# optimisation for communications where security isn't important
$drb_hack_ignore_taint = true

# raises an error if a DRbObject is passed to DRbObject.new, which quickly results in recycled
# object errors after the other end uses it due to DRbObjects' transience.
$enforce_no_drb_object_creation_from_other_drb_objects = true

$performance_impact_options_noncritical = [
  :$enforce_no_drb_object_creation_from_other_drb_objects,
  :$drb_debug_caller_info_in_send_request_exception,
  :$drb_idconv_debug
]

$performance_impact_options = $performance_impact_options_noncritical + [
  :$drb_fix_undumped_array,
]

$performance_enhance_options = [
  :$drb_hack_ignore_taint
]

if $drb_fix_here_equivalence_with_getaddress
  require 'uri'
end

require 'monitor' 

if $drb_no_drb_respond_to || $drb_object_comparable
  class Object
    if $drb_object_comparable
      def drb_equal?(rhs)
        equal?(rhs)
      end
    end

    if $drb_no_drb_respond_to
      def no_drb_respond_to?(*args)
        respond_to?(*args)
      end
    end
    
    if $drb_authoritive_new
      # A block must be supplied. The constructed object is guaranteed not to be GCed for the 
      # duration of the block. If a block must be passed to the class constructor, use
      # authoritive_new_with_block.
      def self.authoritive_new(*args, &block)
        instance = DRb.send_authoritive(true, self, :new, *args)
        yield instance
        DRb.release_authoritive(instance)
      end
      
      # As with authoritive_new, but a block can be passed to the constructor via a proc
      # object.
      def self.authoritive_new_with_block(proc_var, *args, &block)
        instance = DRb.send_authoritive(true, self, :new, *args, &proc_var)
        yield instance
        DRb.release_authoritive(instance)
      end
    end
    
    if $drb_ensure_authoritive
      # Ensures that objects will always present an authoritive interface to the client,
      # by extending it with DRbUndumped.
      def ensure_authoritive
        DRb.make_undumped(self)
      end
    end
  end
end

# tbd: move to AttributeReference_drb? or just stick it in AttributeReference
module AttributeReference
  class Base
    # Ensures that the object is retrieved in such a way that changes made to the object will persist
    # on both client and server. This is appropriate for an operation such as displaying the object
    # in a view that allows modification, but read-only access such as a view that only visualises the
    # object can use the plain 'get' call as an optimisation, or to ensure immutability.
    # This was required because I decided to dump small arrays as an optimisation; it reduces the number
    # of rpcs in that scenario. However, an undumped array is preferable when the client wants to
    # modify to the server's object.  
    def get_authoritive(host=@host)
      if DRb.instance_here?(host)
        get(host)
      else
        get(DRb::DRbObjectWithUndumpedRPC.from_drbobject(host))
      end
    end
  end
  
  class Reader
    def get_authoritive(host=@host)
      # this code was copied because only the "send" call need return an authoritive result.
      if DRb.instance_here?(host)
        get(host)
      else
        raise "No host supplied to either 'get' or 'initialize'." if !host
        
        # xdrb
        if host.respond_to?(:viewable_class)
          raise "Host of class '#{host.viewable_class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
        else
          raise "Host of class '#{host.class}' has no reader called '#{symbol()}'." if !Reader.suitsSymbol?(host, symbol())
        end
  
        host.method_missing_undumped_result(false, symbol())
      end
    end
  end 

  class ArrayElementSymbol < Reader
    # tbd: this is duplicated 'get' code from ArrayElementSymbol. It's needed because it derives
    # from Reader. get_authoritive's implementation differs from 'get' in Reader. Reader's
    # get_authoritive implementation is not called via the usual 'get' call unless this code
    # is here.
    def get_authoritive(host=@host)
      super(host)[@idx]
    end
  end
  
  class ObjectReadOnly
    def get(host=nil)
      raise "The only appropriate host for this object is given on creation." if host
      @host
    end

    def get_authoritive(host=@host)
      # I'm not sure if this needs to be done. Maybe the reference will just always be a DRbObject
      raise "I'm not sure if this needs to be done. Maybe the reference will just always be a DRbObject" if !@host.class == DRb::DRbObject
      @host.instance_eval('self')
    end
  end
end
  
module DRb
  include Abstract
  include ModuleLogging
  
  # It's only appropriate to extend an instance with Undumped when it's a class type, and
  # the type's typical usage suits undumped.  
  def self.is_undumped_appropriate?(obj)
    !obj.nil? \
      && !obj.kind_of?(Numeric) \
      && obj.class != TrueClass \
      && obj.class != FalseClass \
      && obj.class != Proc \
      && obj.class != DRbObject \
      && obj.class != Class
  end
  
  def self.make_undumped(object)
    # tbd: this is to try and ensure that every object created by the method call is kept.
    # a better way might be to add a method to Object that is overridden by array, hash, etc.
    if object.kind_of?(Array) || object.kind_of?(Hash)
      object.each do |x|
        make_undumped(x)
      end
    end
    
    object.extend(DRb::DRbUndumped) if is_undumped_appropriate?(object)
    object
  end
  
  if $drb_authoritive_send
    def self.remote_authority=(drb_object)
      @remote_authority = drb_object
      remote_authority_methods_eval_str = <<-EOS
        self.class.module_eval do
					if !method_defined?(:verify_authoritive_uri_ok)
						def verify_authoritive_uri_ok(drb_object_uri)
              if not DRb.here?(drb_object_uri)
                raise "The uri '\#\{drb_object_uri\}' should be considered local to the server, but isn't. This will prevent the proper operation of DRb. Server uri is '#{DRb.current_server.uri}'."
              end
            end
					end

          if !method_defined?(:send_authoritive)
            def send_authoritive(force_keep_result_reference, instance, symbol, *args, &block)
              raise "This method must operate on an authoritive object originating on the server (object class \#{instance.class}, id \#{instance.object_id})." if !DRb.instance_here?(instance)
              
              result = instance.send(symbol, *args, &block)
              
              undumped_is_appropriate = DRb.is_undumped_appropriate?(result)
              
              if undumped_is_appropriate
                prevent_gc_authoritive(result) if force_keep_result_reference
                result = DRb::DRbObjectWithUndumpedRPC.from_instance(result)
              end
              
              result
            end
            
            def prevent_gc_authoritive(object)
              return nil if object.nil?
              
              debugger if !DRb.instance_here?(object)
              raise "This method must operate on an authoritive object originating on the server (object class \#{object.class}, id \#{object.object_id})." if !DRb.instance_here?(object)
              
              @drb_authoritive_refs ||= {}
              
              # tbd: this is to try and ensure that every object created by the method call is kept.
              # a better way might be to add a method to Object that is overridden by array, hash, etc. 
              if object.kind_of?(Array) || object.kind_of?(Hash)
                object.each do |x|
                  prevent_gc_authoritive(x)
                end
              end
              
              tuple = @drb_authoritive_refs[object.object_id]
              
              if tuple
                tuple[1] += 1
              else
                @drb_authoritive_refs[object.object_id] = [object, 1]
              end
              
              nil
            end
          
            def release_authoritive(instance)
              return nil if instance.nil?
              
              tuple = @drb_authoritive_refs[instance.object_id]
              
              if !@drb_authoritive_refs || !tuple
                $stderr.puts("release_authoritive: No reference is current held to object with id '\#\{instance.object_id\}', this could indicate an error.")
                $stderr.puts caller
                return nil
              end
              
              if tuple.last <= 0
                $stderr.puts("release_authoritive: Attempt to decrement a reference count below 0 for object with id '\#\{instance.object_id\}'.")
                $stderr.puts caller
                return nil
              end
          
              tuple[1] -= 1
                    
              if tuple.last <= 0 
                @drb_authoritive_refs.delete(instance.object_id)
              end
            end
          end
        end
      EOS

      log { "Setting remote authority to '#{drb_object.__drburi}'" }

      # Construct methods on the remote side. An eval string must be used, otherwise blocks would execute
      # locally.
      @remote_authority.send(:method_missing, :instance_eval, remote_authority_methods_eval_str)

      # Send the authority a DRbObject of itself to verify that the URIs are set correctly.
      @remote_authority.verify_authoritive_uri_ok(drb_object.__drburi)

      log { "Remote authority set to '#{drb_object.__drburi}'" }
    end
    
    def self.send_authoritive(force_keep_result_reference, instance, symbol, *args, &block)
      return instance.send(symbol, *args, &block) if !@remote_authority
      result = @remote_authority.send_authoritive(force_keep_result_reference, instance, symbol, *args, &block)
      result
    end
  
    def self.release_authoritive(instance)
      return if !@remote_authority
      @remote_authority.release_authoritive(instance)
    end
    
    def self.prevent_gc_authoritive(object)
      return if !@remote_authority
      @remote_authority.prevent_gc_authoritive(object)
    end
  end
  
  if $drb_specific_dump
    class DRbSpecificDumpDataWrapper
      def initialize(dump_instance, depth)
        @data = Marshal.dump([dump_instance.class, dump_instance._drb_dump(depth)])
      end
      
      def _dump(depth)
        @data
      end
      
      def self._load(str)
        data = Marshal.load(str)
        load_class = data.first 
        load_class._drb_load(data.last)
      end
    end
    
    module DRbSpecificDump
      include Abstract
      include IncludableClassMethods
      
      module ClassMethods
        def _drb_load(str)
          abstract
        end
      end
      
      def drb_dump(depth)
        Marshal.dump(DRbSpecificDumpDataWrapper.new(self, depth))
      end
      
    :protected
      def _drb_dump(depth)
        abstract
      end  
    end
  end
  
  if $drb_module_never_undumped
    module DRbNeverUndumped
    end
  end
  
  # A type of DRbObject that ensures all method call results are DRbObjectWithUndumpedRPCs too.
  # Ensures that DRbUndumped method calls behaviour is execute, while not requiring the object
  # to be extended with undumped.
  class DRbObjectWithUndumpedRPC < DRbObject
    protected :initialize
    
    def self.from_drbobject(drbobject)
      raise "Method must be supplied with a DRbObject instance." if !drbobject.kind_of?(DRb::DRbObject)
      # Another option would be to delegate calls to DRbObject, but I think it's important that
      # class be a kind of DRbObject. That concern might be unfounded.
      # Yet another alternative would be to override kind_of?. But that
      # seems like a bad idea, code shouldn't violate the user's assumptions about kind_of's result.
      DRbObjectWithUndumpedRPC.new_with(drbobject.__drburi, drbobject.__drbref)
    end
    
    def self.from_instance(instance)
      raise "Method must be supplied with a non-DRbObject instance." if instance.kind_of?(DRb::DRbObject)
      DRbObjectWithUndumpedRPC.new(instance)
    end
    
    def self.from_instance_or_drbobject(drbobject_or_instance)
      if drbobject_or_instance.kind_of?(DRb::DRbObject)
        from_drbobject(drbobject_or_instance)
      else
        from_instance(drbobject_or_instance)
      end
    end
    
    def method_can_force_undumped?(symbol)
      symbol != :to_str
    end
    
    def method_missing(symbol, *args, &block)
      if method_can_force_undumped?(symbol)
        method_missing_undumped_result(false, symbol, *args, &block)
      else
        super
      end
    end    
  end
        
  if $drb_fix_here_equivalence_with_getaddress
    def normalized_uri_host(uri)
      uri = if uri.kind_of?(String)
        begin
          URI.parse(uri)
        rescue Exception=>e
          $stderr.puts "DRb.normalized_uri_host: Exception trying to parse uri '#{uri}': #{e}"
          $stderr.puts caller
          raise
        end
      else
        uri
      end 
      # to solve issues with various linux programs that expect hostnames to map to fully qualified
      # and canonical domain names, /etc/hosts often contains a mapping from the machine name to 127.0.1.1.
      # this should be considered equivalent to 127.0.0.1. 
      # http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=316099
      # http://lists.debian.org/debian-boot/2005/06/msg00639.html
      
      result = IPSocket.getaddress(uri.host)
      result = "127.0.0.1" if result == "127.0.1.1"
      result
    end
    module_function :normalized_uri_host
    
    def here?(uri)
      if (current_server.uri rescue nil) == uri
        true
      elsif !uri.empty?
        @uri_here_cache ||= {}
          
        is_here = @uri_here_cache[uri]
        
        if is_here == nil
          server_parsed = begin
            URI.parse(current_server.uri)
          rescue Exception=>e
            $stderr.puts "DRb.here?: Exception trying to parse current server uri '#{current_server.uri}': #{e}"
            $stderr.puts caller
            raise
          end
          
          uri_parsed = begin
            URI.parse(uri)
          rescue Exception=>e
            $stderr.puts "DRb.here?: Exception trying to parse object uri '#{uri}': #{e}"
            $stderr.puts caller
            raise
          end
            
          resolved_server = normalized_uri_host(server_parsed)
          resolved_uri = normalized_uri_host(uri_parsed)
          
          is_here = resolved_server == resolved_uri && server_parsed.port == uri_parsed.port
            
          @uri_here_cache[uri] = is_here 
        end
        
        is_here
      else
        false
      end
    end
    module_function :here?
  end
  
  if $drb_instance_here
    def instance_here?(uri)
      return true if uri.nil?
      
      if uri.kind_of?(DRbObject)
        return here?(uri.__drburi)
      else
        return true
      end
    end
    module_function :instance_here?
  end
  
  if $drb_extra_safety
    # Anything that would cause certain classes to fail to dump, such as the addition of mixins,
    # will cause issues such as endless send/reply loops or obscure drb failures such as nil uris in
    # drb requests. This should be prevented.
    NilClass.freeze
    Proc.freeze
  end
  
  if $drb_idconv_debug
    class DRbIdConv
      @@map = {} # dlb
      def to_obj(ref)
        begin
          result = ObjectSpace._id2ref(ref)
          @@map[ref] = result.class
          result 
        rescue
          $stderr.puts "#{self.class}: error for ref '#{ref}', last successful conversion for that id was object of type '#{@@map[ref]}'"
          raise
        end
      end
      
      def to_id(obj)
        @@map[obj.__id__] = obj.class
        obj.nil? ? nil : obj.__id__
      end
    end
  end

  if $drb_singlethreaded_blocks
    class DRbBlockProxy
      def initialize(block, force_undumped_result)
        @block = block
        @force_undumped_result = force_undumped_result
      end
      
      def on_client_received(message, stream)
        @message = message
        @stream = stream
      end
      
      def call(*args)
        if @message
          @message.send_reply(@force_undumped_result, @stream, DRbSinglethreadedBlockResponse, args)
          @message.recv_reply(@stream)
        else
          @block.call(*args)
        end
      end
      
      def post_perform
        @message = nil
        @stream = nil
      end
      
      def _dump(depth)
        Marshal.dump([DRbObject.new(@block), @force_undumped_result])
      end
      
      def self._load(str)
        self.new(*Marshal.load(str))
      end
    end
  end
    
  class DRbServer
    if $drb_method_missing_undumped_result_option
      def main_loop
        Thread.start(@protocol.accept) do |client|
          @grp.add Thread.current
          Thread.current['DRb'] = { 'client' => client ,
            'server' => self }
          loop do
            begin
              succ = false
              invoke_method = InvokeMethod.new(self, client)
              succ, result, force_undumped_result = invoke_method.perform
              if !succ && verbose
                p result
                result.backtrace.each do |x|
                  puts x
                end
              end
              client.send_reply(succ, result, force_undumped_result) rescue nil
            ensure
              client.close unless succ
              if Thread.current['DRb']['stop_service']
                Thread.new { stop_service }
              end
              break unless succ
            end
          end
        end
      end
    end
  
    class InvokeMethod
        #this was used to debug the weird occurances of DRbObjects that was happening because of the
        #failing DRb.here equivalence test 
#        def perform
#          @result = nil
#          @succ = false
#          setup_message
#
#          if $SAFE < @safe_level
#            info = Thread.current['DRb']
#            if @block
#              @result = Thread.new {
#                Thread.current['DRb'] = info
#                $SAFE = @safe_level
#                perform_with_block
#              }.value
#            else
#              @result = Thread.new {
#                Thread.current['DRb'] = info
#                $SAFE = @safe_level
#                perform_without_block
#              }.value
#            end
#          else
#            if @block
#              if @block.class == DRbObject
#                xx "ERROR"
#                puts caller
#                xx '-----'
#                xx @block.class
#                xx @block
#                exit
#              end
#              @result = perform_with_block
#            else
#              @result = perform_without_block
#            end
#          end
#          @succ = true
#          if @msg_id == :to_ary && @result.class == Array
#            @result = DRbArray.new(@result)
#          end
#          return @succ, @result
#        rescue StandardError, ScriptError, Interrupt
#          @result = $!
#          return @succ, @result
#        end
        
      alias_method :perform_old, :perform
      def perform
        if $drb_improved_exception_handling_on_perform
          succ, result = begin
            perform_old()
          rescue Exception=>e
            $stderr.puts "(#{$0}) Error while executing drb call, message id '#{@msg_id}'"
            $stderr.puts e.class
            $stderr.puts e
            $stderr.puts e.backtrace
            raise
          end
        else
          succ, result = perform_old()
        end
        
        if $drb_method_missing_undumped_result_option
          return succ, result, @force_undumped_result
        else
          return succ, result
        end
      end
      
      if $drb_singlethreaded_blocks == true
        alias_method :perform_with_block_old, :perform_with_block
        def perform_with_block
          result = perform_with_block_old()
          
          @block.post_perform
          
          result
        end
      end
      
      if $drb_method_missing_undumped_result_option
        def init_with_client
          force_undumped_result, obj, msg, argv, block = @client.recv_request
          @force_undumped_result = force_undumped_result
          @obj = obj
          @msg_id = msg.intern
          @argv = argv
          @block = block
        end

        private :init_with_client
      end
    end
  end
  
  if $drb_singlethreaded_blocks || $drb_method_missing_undumped_result_option
    class DRbMessage
      if $drb_fix_undumped_array
        def fix_undumped_array(array)
          result = array.collect do |e|
            if e.class == Array
              fix_undumped_array(e)
            elsif DRbMessage.should_make_undumped(e, true)
              DRb::DRbObject.new(e)
            else
              e
            end
          end
          
          result 
        end
      end
      
      if $drb_method_missing_undumped_result_option
        def send_reply(force_undumped_result, stream, succ, result)
          #xx_outline "from #{@msg}, #{result.object_id}, FORCEDUMP #{force_undumped_result}, SHOULDMAKEDUMP #{DRbMessage.should_make_undumped(result, force_undumped_result)}", result
          stream.write(dump(succ) + dump(result, !succ, force_undumped_result))
        end
      end
      
      def self.should_make_undumped(obj, force_undumped_result)
        DRb.is_undumped_appropriate?(obj) &&
        (obj.kind_of?(DRbUndumped) || 
          ($drb_method_missing_undumped_result_option && force_undumped_result))
      end
      
      def dump(obj, error=false, force_undumped_result=false)  # :nodoc:
        str = if $drb_singlethreaded_blocks && obj.class == Proc
          Marshal::dump(DRbBlockProxy.new(obj, force_undumped_result))
        else
          #xx "DUMPING #{@msg} result #{obj.object_id}"
          if DRbMessage.should_make_undumped(obj, force_undumped_result)
            obj = make_proxy(obj, error)
            #xxe "#{@msg} #{obj.object_id} RESULT MADE PROXY"
          end
          
          begin
            if !obj.no_drb_respond_to?(:drb_dump)
              Marshal::dump(obj)
            else
              obj.drb_dump(-1)
            end
          rescue Exception=>e
            #xxe "#{@msg} #{obj.object_id} RESULT DUMP FAIL #{e}, #{e.backtrace.first}"
            raise "Attempted to make a DRbObject for an instance including DRbNeverUndumped (#{obj.class}, #{obj.object_id}): #{e.class}, #{e}, \n#{e.backtrace.join($\)}" if obj.kind_of?(DRbNeverUndumped)

            if $drb_fix_undumped_array
              if obj.class == Array
                #xx_outline "TRYING TO PREVENT UNDUMPED ARRAY #{obj.object_id}", obj
                obj = fix_undumped_array(obj)
                #xx_outline "FIXED ARRAY #{obj.object_id}", obj
                result = Marshal::dump(obj)
                #xx_outline "DUMPED ARRAY #{obj.object_id}", obj
                result
              else
                proxy = make_proxy(obj, error)
                #xxe "#{@msg} #{obj.object_id} MADE PROXY #{proxy.object_id}"
                Marshal::dump(proxy)
              end
            else
              proxy = make_proxy(obj, error)
              #xxe "#{@msg} #{obj.object_id} MADE PROXY #{proxy.object_id}"
              Marshal::dump(proxy)
            end
          end
        end
        
        [str.size].pack('N') + str
      end
      
      if $drb_method_missing_undumped_result_option
        def send_request(stream, force_undumped_result, ref, msg_id, arg, b) # :nodoc:
          ary = []
            
          if $drb_debug_caller_info_in_send_request_exception
            ary.push(dump(caller()))
          end
          
          ary.push(dump(force_undumped_result))
          ary.push(dump(ref.__drbref))
          ary.push(dump(msg_id.id2name))
          ary.push(dump(arg.length))
          arg.each do |e|
            ary.push(dump(e))
          end
          ary.push(dump(b))
          stream.write(ary.join(''))
        rescue
          raise(DRbConnError, $!.message, $!.backtrace)
        end
      end

      def recv_request(stream) # :nodoc:
        begin
          if $drb_debug_caller_info_in_send_request_exception
            @caller_ary = load(stream)
          end
          
          force_undumped_result = if $drb_method_missing_undumped_result_option
            load(stream)
          else
            false
          end
  
          ref = load(stream)
          msg = load(stream)
          @msg = msg # xdrb delete me
          if $drb_debug_recv_request_trace
            xx "-- START --"
            xx @msg
            xx @caller_ary
            xx "END"
          end          
          ro = DRb.to_obj(ref) # changed order here so msg is available when recycled object error occurs.
          argc = load(stream)
          raise ArgumentError, 'too many arguments' if @argc_limit < argc
          argv = Array.new(argc, nil)
  
          argc.times do |n|
            argv[n] = load(stream)
          end
          
          block = load(stream)
          
          if $drb_extra_debugging && !block.nil? && block.class != DRbBlockProxy
            $stderr.puts "Recieved blocks should always be of class DRbBlockProxy. Something went wrong."
            $stderr.puts "ref         : #{ref}"
            $stderr.puts "ro          : #{ro}"
            $stderr.puts "msg         : #{msg}"
            $stderr.puts "argc        : #{argc}"
            $stderr.puts "argv        : #{argv}"
            $stderr.puts "block class : #{block.class}"
            begin
              $stderr.puts "block       : #{block}"
            rescue Exception=>e
              $stderr.puts "Error printing block class: #{e}"
            end
            
            if @caller_ary
              $stderr.puts "caller:"
              $stderr.puts caller_ary
            end
            
            exit!
          end
          
          block.on_client_received(self, stream) if block
          
          if $drb_method_missing_undumped_result_option
            return force_undumped_result, ro, msg, argv, block
          else
            return ro, msg, argv, block
          end
        rescue Exception=>e
          if $drb_extra_debugging
            $stderr.puts "Exception while processing DRb message at #{DRb.current_server.uri}."
            $stderr.puts "Error       : #{e}"
            $stderr.puts "ref         : #{ref}" rescue nil
            $stderr.puts "msg         : #{msg}" rescue nil
            $stderr.puts "argc        : #{argc}" rescue nil
            $stderr.puts "argv        : #{argv}" rescue nil
            $stderr.puts "block class : #{block.class}" rescue nil
            begin
              $stderr.puts "block       : #{block}"
            rescue Exception=>e
              $stderr.puts "Error printing block class: #{e}"
            end
            
            if @caller_ary
              $stderr.puts "caller:"
              $stderr.puts caller_ary
            end
            
            raise
          else
            raise
          end
          
          @caller_ary = nil
        end
      end
      
      def load(soc)  # :nodoc:
        begin
          sz = soc.read(4)  # sizeof (N)
        rescue
          raise(DRbConnError, $!.message, $!.backtrace)
        end
        raise(DRbConnError, 'connection closed') if sz.nil?
        raise(DRbConnError, 'premature header') if sz.size < 4
        sz = sz.unpack('N')[0]
        raise(DRbConnError, "too large packet #{sz}") if @load_limit < sz
        begin
          str = soc.read(sz)
        rescue
          raise(DRbConnError, $!.message, $!.backtrace)
        end
        raise(DRbConnError, 'connection closed') if str.nil?
        raise(DRbConnError, 'premature marshal format(can\'t read)') if str.size < sz
        
        if $drb_hack_ignore_taint
          begin
            save = Thread.current[:drb_untaint]
            Thread.current[:drb_untaint] = []
            Marshal::load(str)
          rescue NameError, ArgumentError => e
            DRbUnknown.new($!, str)
          end
        else
          DRb.mutex.synchronize do
            begin
              save = Thread.current[:drb_untaint]
              Thread.current[:drb_untaint] = []
              Marshal::load(str)
            rescue NameError, ArgumentError => e
              DRbUnknown.new($!, str)
            ensure
              Thread.current[:drb_untaint].each do |x|
                x.untaint
              end
              Thread.current[:drb_untaint] = save
            end
          end
        end
      end
    end
  end
  
  class DRbSinglethreadedBlockResponse
  end

  if $drb_method_missing_undumped_result_option
    class DRbConn
      def send_message(force_undumped_result, ref, msg_id, arg, block)  # :nodoc:
        @protocol.send_request(force_undumped_result, ref, msg_id, arg, block)
        @protocol.recv_reply
      end
    end

    class DRbTCPSocket
      def send_reply(succ, result, force_undumped_result)
        @msg.send_reply(force_undumped_result, stream, succ, result)
      end
      
      def send_request(force_undumped_result, ref, msg_id, arg, b)
        @msg.send_request(stream, force_undumped_result, ref, msg_id, arg, b)
      end
    end
  end
  
  class ViewableDependencyIdDRbObject < ViewableDependencyRecordSpecialization
    # <=> and hash have potential issues. Two drbobjects may have both the same ref and different but
    # equivalent uris. In this case they are equivalent, but will not be detected as such because
    # the uris differ in content. This is another argument for fixing DRb.here? using a server guid. 
    include Comparable
    include DRbNeverUndumped
     
    def initialize(subject)
      @drburi = subject.__drburi.dup
      @drbref = subject.__drbref
    end
    
    def <=>(other)
      if other.class == self.class
        result = @drbref <=> other.__drbref
        
        if result == 0
          result = DRb.normalized_uri_host(@drburi) <=> DRb.normalized_uri_host(other.__drburi)
        end
        
        result
      else  
        super
      end
    end
    
    def resolve
      DRbObject.new_with(@drburi, @drbref)
    end
    
    def hash
      # cached for speed
      @hash ||= (@drbref.to_s + DRb.normalized_uri_host(@drburi)).hash
    end
    
    def to_s
      "#{self.class} uri:#{@drburi} ref:#{@drbref}"
    end
  end
  
  class DRbUnknown
    def to_s
      "#{super()}: unknown class named '#{@name.inspect}'"
    end
  end

  class DRbObject
if $enforce_no_drb_object_creation_from_other_drb_objects 
    def initialize(obj, uri=nil)
      raise "Attempt to make a DRbObject from another DRbObject" if obj.kind_of?(DRbObject)
      
      @uri = nil
      @ref = nil
      if obj.nil?
        return if uri.nil?
        @uri, option = DRbProtocol.uri_option(uri, DRb.config)
        @ref = DRbURIOption.new(option) unless option.nil?
      else
        @uri = uri ? uri : (DRb.uri rescue nil)
        @ref = obj ? DRb.to_id(obj) : nil
      end
    end
end
    
    # xx DELETE THIS
    def _dump(lv)
      if @uri == nil
        xxe "----------------------"
        xxe "DUMPING NIL URI!!!!!!"
        xxe [@uri, @ref]
        puts caller
        xxe "- END ----------------------"
      end
      
      Marshal.dump([@uri, @ref])
    end
    
    if $drb_object_comparable
      include Comparable
       
      def <=>(other)
        if other.class == self.class
          result = @drbref <=> other.drbref
          if result == 0
            @drburi <=> other.__drburi
          end
        else
          super
        end
      end
      
      # <=>, hash and this equality test have potential issues. Two drbobjects may have both the same 
      # ref and different but equivalent uris. In this case they are equivalent, but will not be 
      # detected as such because the uris differ in content. 
      # This is another argument for fixing DRb.here? using a server guid. 
      def self.drb_equal?(lhs_ref, lhs_uri, rhs_ref, rhs_uri)
        lhs_ref == rhs_ref && lhs_uri == rhs_uri
      end
      
      def drb_equal?(rhs)
        if rhs.class == self.class
          # rhs is a DRbObject
          self.class.drb_equal?(@ref, @uri, rhs.__drbref, rhs.__drburi)
        else
          # rhs is not a DRbObject.
          # drbobjects with a uri matching the local drb server should never exist, because they
          # should have been resolved to real objects. So if rhs is not a drbobject then it must
          # be a local object, and there's no possiblity of a match with this object, which must
          # be remote.
          false  
        end
      end
      
      def hash
        (@ref.to_s + @uri.to_s).hash
      end
    end
    
    def viewable_make_dependant_finalizer_if_required(finalizer_proc)
      method_missing(:viewable_make_dependant_finalizer_if_required, finalizer_proc)
    end
    
    if $drb_no_drb_respond_to
      def no_drb_respond_to?(msg_id, priv=false)
        respond_to?(msg_id, priv, true)
      end
      
      def respond_to?(msg_id, priv=false, no_drb=false)
        return super(msg_id, priv) if no_drb
        
        case msg_id
        when :_dump
          true
        when :marshal_dump
          false
        else
          method_missing(:respond_to?, msg_id, priv)
        end
      end
    end

    def viewable_dependency_id
      # Cached for speed
      @viewable_dependency_id ||= ViewableDependencyIdDRbObject.new(self)
    end
    
    if $drb_drbobject_to_yaml_warn
      def to_yaml(*args)
        $stderr.puts("ERROR: Trying to make yaml from a drbobject. Contents: #{inspect}")
        $stderr.puts("Attempting to get yaml data from remote.")
        remote_result = method_missing(:to_yaml, *args)
        $stderr.puts("Result:\n#{remote_result}")
        remote_result
      end
    end
    
    if $drb_method_missing_undumped_result_option
      def handle_singlethreaded_block_result(conn, b, msg_succ, msg_result)
        if msg_succ == DRbSinglethreadedBlockResponse
          # the server has evaluated code and returned a value to be yielded to the client's block
          protocol = conn.instance_variable_get(:@protocol)
           
          begin
            raise "msg_result must always be an array, was #{msg_result.class}" if msg_result.class != Array
            # the client evaluates local code and sends the result back to the server
            block_result = b.call(*msg_result)
            protocol.send_reply(true, block_result, true)
            msg_succ, msg_result = protocol.recv_reply
          end while msg_succ == DRbSinglethreadedBlockResponse
        end
        
        return msg_succ, msg_result
      end
      
      # i copied the code rather than refactoring to avoid code duplication for now because i don't 
      # know whether introducting another method call would negatively impact drb performance
      # significantly. 
      def method_missing_undumped_result(all_subsequent_calls_undumped, msg_id, *a, &b)
        if DRb.here?(@uri)
          obj = DRb.to_obj(@ref)
          DRb.current_server.check_insecure_method(obj, msg_id)
          return obj.__send__(msg_id, *a, &b)
        end
    
        succ, result = self.class.with_friend(@uri) do
          DRbConn.open(@uri) do |conn|
            # this is the client requesting a method call from the server, optionally passing a block
            msg_succ, msg_result = conn.send_message(true, self, msg_id, a, b)
            
            #xxe "METHOD_MISSING GOT DRBOBJECT FROM #{msg_id}" if msg_result.kind_of?(DRbObject)
            
            msg_succ, msg_result = handle_singlethreaded_block_result(conn, b, msg_succ, msg_result)
            
            # ensure that a DRbObjectWithUndumpedRPC object is sent back so that 
            # subsequent calls will also return undumped results.
            if msg_succ \
              && all_subsequent_calls_undumped \
              && (msg_result.kind_of?(DRbObject) || DRb.is_undumped_appropriate?(msg_result))
              msg_result = DRbObjectWithUndumpedRPC.from_instance_or_drbobject(msg_result)
            end
            
            [msg_succ, msg_result]
          end
        end
    
        if succ
          return result
        elsif DRbUnknown === result
          raise result
        else
          bt = self.class.prepare_backtrace(@uri, result)
          result.set_backtrace(bt + caller)
          raise result
        end
      end
    end
    
    if $drb_instance_variable_get_distribute
      def instance_variable_get(*args)
        method_missing(:instance_variable_get, *args)
      end
    end
    
    if $drb_comparable_distribute
      def <=>(rhs)
        method_missing(:<=>, rhs)
      end
    end
    
    def method_missing(msg_id, *a, &b)
      # xx delete this
      if @uri == nil
        xx "NIL URI (#{@ref})!!!!!!"
        puts caller
      end
      
      if DRb.here?(@uri)
        obj = DRb.to_obj(@ref)
        DRb.current_server.check_insecure_method(obj, msg_id)
        return obj.__send__(msg_id, *a, &b)
      end
  
      succ, result = self.class.with_friend(@uri) do
        DRbConn.open(@uri) do |conn|
          # this is the client requesting a method call from the server, optionally passing a block
          msg_succ, msg_result = if $drb_method_missing_undumped_result_option
            conn.send_message(false, self, msg_id, a, b)
          else
            conn.send_message(self, msg_id, a, b)
          end

          #xxe "METHOD_MISSING GOT DRBOBJECT FROM #{msg_id}" if msg_result.kind_of?(DRbObject)
          
          msg_succ, msg_result = handle_singlethreaded_block_result(conn, b, msg_succ, msg_result)
          
          [msg_succ, msg_result]
        end
      end
  
      if succ
        return result
      elsif DRbUnknown === result
        raise result
      else
        bt = self.class.prepare_backtrace(@uri, result)
        result.set_backtrace(bt + caller)
        raise result
      end
    end
  end
  
  if $drb_debug_incoming
    class DRbMessage
      alias_method :old_recv_request, :recv_request
      def recv_request(stream)
        ro, msg, argv, block = old_recv_request(stream)
        if Thread.current != DRb.thread
          xx 'Not DRb'
        end
        # this call makes wx and possibly drb calls and slows everything down.
        # wx cross-thread detection is not foolproof, beware.
        #xx ro
        xx msg
        xx argv
        xx block
        return ro, msg, argv, block
      end
    end
  end
  
  def self.ensure_drb_object(obj_or_drb_object)
    if obj_or_drb_object.kind_of?(DRbObject)
      obj_or_drb_object
    else
      DRbObject.new(obj_or_drb_object)
    end
  end
end

class Viewable::ModificationRecord
  include DRb::DRbNeverUndumped
  include DRb::DRbSpecificDump

  def drb_data
    [DRb.ensure_drb_object(@who), DRb.ensure_drb_object(@instigator)]
  end
  
  def _drb_dump(depth)
    Marshal.dump(drb_data())
  end
  
  def self._drb_load(str)
    load_data = Marshal.load(str)
    who = load_data.shift
    result = self.new(*load_data)
    result.who = who
    result
  end
end

class ArrayModificationRecord < Viewable::ModificationRecord
  def drb_data
    super + [@indexes, @action]
  end
end
