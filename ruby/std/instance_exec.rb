if RUBY_VERSION >= "1.9"
  # $stderr.puts "instance_exec: This functionality is already included in language version 1.9."
else
  # http://www.ruby-forum.com/topic/54096
  class Object
    def instance_exec(*args, &block)
      mname = "__instance_exec_#{Thread.current.object_id.abs}"
      begin
        class << self; self end.class_eval{ define_method(mname, &block) }
        ret = send(mname, *args)
      ensure
        class << self; self end.class_eval{ undef_method(mname) } rescue nil
      end
      ret
    end
  end
end
