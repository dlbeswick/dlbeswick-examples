require 'std/unique'
require 'std/path'
require 'analysis/relational_db'

RelationalDBWeakRef.log = true
RelationalDBResolver.log = true

def test_dir
  Path.to_dir(Dir.pwd).parent_with_name("Source") + "test"
end

class RelationalDBTestRecord
  include Unique

  auto_id
  use_id_context :app, :records

  persistent_accessor :app
  persistent_accessor :hello
  persistent_accessor :obj_ref
  persistent_accessor :cum

  def self.allocate
    puts "ALLOC #{object_id()}"
    instance_variable_set(:@cum, caller.join)
    super
  end

  def initialize
    puts "CREATE #{object_id()}"
    instance_variable_set(:@cum, caller.join)
  end

  def to_s
    "I'm a test record that says '#{hello}'.\napp: #{app}\nhello: #{hello}\nobj_ref: #{obj_ref}"
  end
end

class RelationalDBTestApp
  include UniqueHost
  include Unique
  
  auto_id
  use_global_id_context

  unique_id_registry RelationalDBTestRecord, :records
end

obj = nil

if ARGV[0] != 'read'
app = RelationalDBTestApp.new
db = RelationalDB.new(test_dir + Path.to_file('relational-db-test.sqlite'))

obj = RelationalDBTestRecord.new
obj.hello = 'hi!'
obj.app = app
app.records.add(obj)
    p '++++++++++++++++++++++++++++++++++++++++++++++++'
db.store(app)

obj2 = RelationalDBTestRecord.new
obj2.hello = 'hi again.'
obj2.obj_ref = obj
obj2.app = app
app.records.add(obj2)
    p '++++++++++++++++++++++++++++++++++++++++++++++++'
db.store(app)

db.close
end

db = RelationalDB.new(test_dir + Path.to_file('relational-db-test.sqlite'))
app = db.retrieve(RelationalDBTestApp, 1, nil)
puts "Result:"
p app
p app.records.ids
p app.records[0].resolve
puts app.records[0]
p app.records[2].resolve
p app.records[2]
app.records[2].obj_ref = nil
puts app.records[2]
puts app.records[2].obj_ref
db.close

obj.app = nil unless obj.nil?
obj = nil
obj2.app = nil unless obj2.nil?
obj2 = nil
app.records._hash.dup.each { |key, val| val.uniqueid_unbind; val.app = nil }
app.records._hash.clear
app.records.host = nil
app.uniqueid_unbind
app = nil
db = nil


module GC
  def self.find_refs_to_class(klass)
    objects = ObjectSpace.each_object.to_a
    klass_s = klass.to_s
    objects.each do |obj|
      next if obj.class == String
      next if obj.class.name.include?('Syck')
      next if obj.eql?(objects)

      s = 
        begin 
          if obj.kind_of?(UniqueIDConfig) # hack
            UniqueIDConfig::ContentWriter.new(obj).to_yaml 
          else
            obj.to_yaml
          end
        rescue TypeError
          '' 
        end
      
      if s.include?(klass_s)
        puts "-------------------";
        if obj.kind_of?(Module)
          puts "MODULE #{obj}"
        else
          puts "INSTANCE OF #{obj.class} (#{obj.object_id})"
        end
        puts s
      end
    end
  end

  def self.verify_and_debug_object_gc(klass)
    remaining_objs = ObjectSpace.each_object(klass).to_a
    if remaining_objs.length != 0
      remaining_objs.each { |obj| puts "#{obj.class}@#{obj.object_id} not garbage collected." }
      puts "Some possible references to instances of that class:"
      GC.find_refs_to_class(klass)
    end
  end
end

puts "GC"
ObjectSpace.garbage_collect
puts "GC end."

GC.verify_and_debug_object_gc(RelationalDBWeakRef)
GC.verify_and_debug_object_gc(RelationalDBTestApp)
GC.verify_and_debug_object_gc(RelationalDBTestRecord)
GC.verify_and_debug_object_gc(RelationalDB)


