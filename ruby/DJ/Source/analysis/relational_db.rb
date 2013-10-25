require 'std/unique'
require 'std/abstract'
require 'std/path'
require 'std/module_logging'
gem 'sqlite3'
require 'sqlite3'
require 'uri'

module URI
  class DBOBJREF < Generic
    def self.from_obj(object)
      if !object.kind_of?(Unique)
        raise "Can only form references to Unique objects."
      end

      new("dbobjref", nil, "self", nil, nil, build_path(object), nil, nil, nil)
    end

    def to_object_and_id_hierarchy
      tokens = path().scan(/\/([^.]+).([^\/]+)/)
      raise "Malformed object ref uri '#{uri}'." if tokens.nil?

      object_and_id_hierarchy = tokens.collect do |token|
        [eval(token.first), self.class.string_to_id(URI.unescape(token.last))]
      end

      if !object_and_id_hierarchy.first[0].unique_has_global_context?
        raise "Root object in hierarchy must use global ids."
      end

      object_and_id_hierarchy
    end

    def self.object_and_id_hierarchy(object)
      if !object.unique_context_host.nil?
        object_and_id_hierarchy(object.unique_context_host) + [[object.class, object.unique_id]]
      else
        [[object.class, object.unique_id]]
      end
    end

    protected
    def self.string_to_id(string)
      # tbd: support non-numeric ids
      string.to_i
    end

    def self.build_path(object)
      result = ''

      object_and_id_hierarchy(object).each do |object_class, unique_id|
        result += "/#{object_class}.#{URI.escape(unique_id.to_s)}"
      end

      result
    end
  end

  class DBHOSTED < Generic
  end
  
  @@schemes['DBOBJREF'] = DBOBJREF
  @@schemes['DBHOSTED'] = DBHOSTED
end

module RelationalDBStorable
  include Abstract
  include IncludableClassMethods

  module ClassMethods
    def relational_db_extra_columns
      []
    end
  end

  def relational_db_extra_column_values
    {}
  end

  def relational_db_store?
    false
  end

  # called if relational_db_store? returns true
  def relational_db_store(relational_db, attr_ref, attr_ref_host, replace)
    abstract
  end

  def relational_db_restore(column_name, values, attr_ref)
    nil
  end

  # Called if relational_db_own_store_handler? returns false.
  # Returns a hash of column names to column values, to be inserted into the row for the object.
  def relational_db_format(attr_ref, db)
    abstract
  end

  def relational_db_resolved?
    true
  end

  def resolve
    self
  end
end

class Array
  include RelationalDBStorable

  def relational_db_store(relational_db, attr_ref, attr_ref_host, replace)
    output_hash = {}
      
    each_with_index() do |item, idx|
      output_hash[:idx] = idx
      output_hash[:value] = item
    end
    
    db.store_flat_data(output_hash, attr_ref, replace)
  end
end

class Hash
  include RelationalDBStorable

  def relational_db_store(relational_db, attr_ref, attr_ref_host, replace)
    output_hash = {}
      
    each() do |key, value|
      output_hash[:key] = key
      output_hash[:value] = value
    end
    
    db.store_flat_data(output_hash, attr_ref, replace)
  end
end

class ContentWriterHash
  def relational_db_store(relational_db, attr_ref, attr_ref_host, replace)
    each_value() do |value|
      db.store(value, attr_ref, replace)
    end
  end
end

class UniqueHost::UniqueRegistry
  include RelationalDBStorable

  def self.relational_db_retrieve_hosted_object(db, host_class, attr_ref)
    table_name = "#{self.name}!#{host_class}"
    rows = db.execute("select #{db.quote('object_kind_for_db')}, #{db.quote('hosted_object')} from #{db.quote(table_name)} where #{db.quote('host_symbol')} = #{db.quotestr(attr_ref.symbol)}")
    
    raise "No rows." if rows.empty?
    
    object_kind = eval(rows.first[0])
    
    result = new(attr_ref.host, eval(rows.first.first))
    
    def result.id_from(id_or_object)
      if id_or_object.kind_of?(RelationalDBWeakRef)
        id_or_object.unique_id
      else
        super
      end
    end

    def result.add(id_or_object, allow_type_conversion = true)
      #tbd: remove code duplication
      id = id_from(id_or_object) 
      raise "Id '#{id}' is in use." if @hash.has_key?(id)
      if id_or_object.kind_of?(RelationalDBWeakRef)
        @hash[id] = id_or_object
      else
        super
      end
    end

    rows.each do |row|
      ref_text = row.last

      hierarchy = URI(ref_text).to_object_and_id_hierarchy
      p hierarchy
      value_object_id = hierarchy.last[1]

      attr_key = AttributeReference::HashKeyElement.new(value_object_id, :hash)
      attr_value = AttributeReference::HashValueElement.new(attr_key)

      result.add(RelationalDBWeakRef.ref_if_not_resolved(attr_value, result, hierarchy, db), false)
    end

    result
  end

  def self.relational_db_initialize(db, table_name, attr_ref)
    # tbd: de-dup object_kind data, store in separate table joined by host_symbol
    rows = db.execute("select #{db.quote('object_kind_for_db')} from #{db.quote(table_name)} where #{db.quote('host_symbol')} = #{db.quotestr(attr_ref.symbol)}")

    raise "No object_kind_for_db results in table '#{table_name}' for host_symbol '#{attr_ref.symbol}'." if rows.empty?

    result = new(attr_ref.host, eval(rows.first.first))
    
    def result.id_from(id_or_object)
      if id_or_object.kind_of?(RelationalDBWeakRef)
        id_or_object.unique_id
      else
        super
      end
    end

    def result.add(id_or_object, allow_type_conversion = true)
      #tbd: remove code duplication
      id = id_from(id_or_object) 
      raise "Id '#{id}' is in use." if @hash.has_key?(id)
      if id_or_object.kind_of?(RelationalDBWeakRef)
        @hash[id] = id_or_object
      else
        super
      end
    end

    result
  end

  def relational_db_store?
    true
  end

  def relational_db_restore(column_name, values, attr_ref)
    case column_name
    when 'hosted_object'
      values.each do |v|
        id = v.unique_id.to_i
        attr_key = AttributeReference::HashKeyElement.new(id, :hash, self)
        attr_value = AttributeReference::HashValueElement.new(attr_key)
        v.attr_ref = attr_value
        add(v, false)
      end
      true
    when 'host_symbol'
      true
    when 'object_kind_for_db'
      true
    else
      super
    end
  end

  def relational_db_store(relational_db, attr_ref, attr_ref_host, replace)
    uniquehost_rows = RelationalDB::RowDefs.new
    uniquehost_rows.add_column(RelationalDB::ColumnDef.new('hosted_object', RelationalDB::ColumnAttributePrimaryKey))
    uniquehost_rows.add_column(RelationalDB::ColumnDef.new('host_symbol'))
    uniquehost_rows.add_column(RelationalDB::ColumnDef.new('object_kind_for_db')) # different name so that attr_ref setting code isn't invoked in restore. hack due to bad table design.

    _hash().values.collect do |val|
      uniquehost_row = {}
      uniquehost_row['hosted_object'] = URI::DBOBJREF.from_obj(val).to_s
      uniquehost_row['host_symbol'] = attr_ref.symbol.to_s
      uniquehost_row['object_kind_for_db'] = object_kind_class().to_s
      uniquehost_rows.add_row(uniquehost_row)

      hosted_object_rows = relational_db.rowdefs_for_config_object(val)
      relational_db._store_flat_data(hosted_object_rows, val.class, attr_ref, replace, nil)
    end

    relational_db._store_flat_data(uniquehost_rows, self.class, attr_ref, replace, "#{attr_ref_host.class}")

    "!hosted-#{self.class}"
  end
end

# The usual method of storing a unique object is by calling RelationalDB.store.
# This code path is executed when storing a reference to a unique object from another unique object.
# The reference id is stored in a column named "ref-<class_name>".
module Unique
  include RelationalDBStorable
  include IncludableClassMethods

  module ClassMethods
    def relational_db_extra_columns
      [RelationalDB::ColumnDef.new('unique_id', RelationalDB::ColumnAttributePrimaryKey)]
    end
  end

  def relational_db_extra_column_values
    {'unique_id'=>unique_id()}
  end

  def relational_db_format(attr_ref, db)
    {RelationalDB::ColumnDef.new(attr_ref.symbol.to_s) => URI::DBOBJREF.from_obj(self).to_s}
  end

  def relational_db_restore(column_name, values, attr_ref)
    if column_name == 'unique_id'
      raise "Too many rows for unique id: #{values}." if values.length != 1

      id = values.first.to_i

      # Ensure host (unique id context) is resolved
p "*()(**(*("
      p unique_context_host()
      p unique_context_host().resolve if unique_context_host
      p unique_context_host()
      unique_context_host().resolve if unique_context_host()

      existing_bound_obj = self.class.object_for_unique_id(id, uniqueid_context(), false)
      if !existing_bound_obj.nil?
        if existing_bound_obj.kind_of?(RelationalDBWeakRef)
          self.class.uniqueid_unbind_id(id, uniqueid_context())
        else
          raise "Id alread bound for '#{self.class}' in host '#{unique_context_host().class}': #{id}"
        end
      end

      p '@$$@$@$@@$$@$@'
      p id
      bind_id(id) # tbd: allow bind of non-integer ids
      p unique_id()
      p object_id()
      puts to_yaml()
      true
    else
      super
    end
  end
end

class RelationalDBWeakRef
  include ModuleLogging
  
  attr_reader :unique_id
  attr_accessor :attr_ref
  attr_accessor :host
  attr_reader :object_and_id_hierarchy

  def self.ref_if_not_resolved(attr_ref, host, uri_or_object_and_id_hierarchy, db)
    object_and_id_hierarchy = 
      case uri_or_object_and_id_hierarchy
      when URI::DBOBJREF
        uri_or_object_and_id_hierarchy.to_object_and_id_hierarchy
      when Array
        uri_or_object_and_id_hierarchy
      else
        raise "Must be URI::DBOBJREF or array of class and id tuples."
      end

    existing_object = db.resolver.find(object_and_id_hierarchy)
    if existing_object
      existing_object
    else
      new(attr_ref, host, object_and_id_hierarchy, db)
    end
  end

  def initialize(attr_ref, host, uri_or_object_and_id_hierarchy, db)
    @object_and_id_hierarchy = 
      case uri_or_object_and_id_hierarchy
      when URI::DBOBJREF
        uri_or_object_and_id_hierarchy.to_object_and_id_hierarchy
      when Array
        uri_or_object_and_id_hierarchy
      else
        raise "Must be URI::DBOBJREF or array of class and id tuples."
      end

    raise "Attribute reference must have nil host." if !attr_ref.host.nil?
    @attr_ref = attr_ref
    @host = host

    @unique_id = @object_and_id_hierarchy.last.last
    @db = db
    @db.resolver.register_weakref(self)
  end

  def relational_db_store?
    raise "Attempt to store weak ref in db."
  end

  def hash
    @unique_id.hash
  end

  def relational_db_resolved?
    false
  end

  def resolve
    object = @db.resolver.find_or_retrieve(@object_and_id_hierarchy)
    if @attr_ref.nil?
      # tbd: needs a rethink, determine if find_or_retrieve is the right action here. being nulled via _resolve_to
      return object
    end
    @attr_ref.set(object, @host)
    # Clear host to enable it to be GCed, and attr_ref for good measure.
    @attr_ref = nil
    @host = nil
    object
  end

  def _resolve_to(instance)
    raise "Already resolved." if @attr_ref.nil?
    @attr_ref.set(instance, @host)
    @attr_ref = nil
    @host = nil
  end

  def to_s
    "#{self.class}: #{@object_and_id_hierarchy.inspect}"
  end
end

class RelationalDBResolver
  include ModuleLogging

  def initialize(db)
    @db = db
    @id_to_object_id = {}
    @id_to_weakref_object_id = {}
  end

  def self.finalizer(hash, hashkey, debug_info)
    proc do
      log { "Stop tracking #{hashkey} (#{debug_info})" }
      hash.remove(hashkey)
    end
  end

  def register_weakref(weakref)
    object_and_id_hierarchy = weakref.object_and_id_hierarchy
    ary = @id_to_weakref_object_id[object_and_id_hierarchy]
    if ary.nil?
      ary = @id_to_weakref_object_id.store(object_and_id_hierarchy, [])
    end

    ary << weakref.object_id

    ObjectSpace.define_finalizer(weakref, self.class.finalizer(@id_to_weakref_object_id, object_and_id_hierarchy, "weakref@#{weakref.object_id}"))
  end

  def find(object_and_id_hierarchy)
    object_ruby_id = @id_to_object_id[object_and_id_hierarchy]

    if object_ruby_id.nil?
      nil
    else
      ObjectSpace._id2ref(object_ruby_id)
    end
  end

  def find_or_retrieve(object_and_id_hierarchy)
    object_ruby_id = find(object_and_id_hierarchy)

    if !object_ruby_id.nil?
      object_ruby_id
    else
      note { "Resolving weakref to #{object_and_id_hierarchy.last.join('.')}" }

      # link occurs in db.retrieve. is this proper?
      @db.retrieve(object_and_id_hierarchy.last.first, object_and_id_hierarchy.last.last, nil, @attr_ref, 'unique_id', false, object_and_id_hierarchy)
    end
  end

  def link(object, link_to_same_is_fatal = true)
    object_and_id_hierarchy = URI::DBOBJREF.object_and_id_hierarchy(object)

    already_linked_object_id = @id_to_object_id[object_and_id_hierarchy]

    raise "Object '#{object_and_id_hierarchy}' already linked to object id '#{already_linked_object_id}'." if !already_linked_object_id.nil? && already_linked_object_id != object.object_id && link_to_same_is_fatal
    raise "Object '#{object_and_id_hierarchy}' already linked." if !already_linked_object_id.nil?

    log { "Link '#{object_and_id_hierarchy.join('.')}' to object_id #{object.object_id}" }
    @id_to_object_id[object_and_id_hierarchy] = object.object_id
    ObjectSpace.define_finalizer(object, self.class.finalizer(@id_to_object_id, object_and_id_hierarchy, "#{object.class}@#{object.object_id}"))

    weakrefs_for_ref(object_and_id_hierarchy).each do |weakref|
      weakref._resolve_to(object)
    end
  end

  def weakrefs_for_ref(object_and_id_hierarchy)
    ary = @id_to_weakref_object_id[object_and_id_hierarchy]
    return [] if ary.nil?

    ary.collect { |object_id| ObjectSpace._id2ref(object_id) }
  end
end

class RelationalDB
  include ModuleLogging

  class ColumnAttribute
    def self.sql_text(db, column_def)
      abstract
    end
  end

  class ColumnAttributeName
    def self.sql_text(db, column_def)
      db.quote(column_def.name.to_s)
    end
  end

  class ColumnAttributePrimaryKey < ColumnAttribute
    def self.sql_text(db, column_def)
      "primary key"
    end
  end

  class ColumnDef
    include Comparable

    attr_reader :name
    attr_reader :attributes

    def initialize(name, *attributes)
      @attributes = [ColumnAttributeName] + attributes
      raise "Name must be a string." if name.class != String
      @name = name
    end

    def <=>(rhs)
      if rhs.kind_of?(ColumnDef)
        if eql?(rhs)
          0
        else
          name() <=> rhs.name
        end
      elsif rhs.kind_of?(String)
        name() <=> rhs
      else
        super
      end
    end

    def hash
      name().hash
    end

    def sql_text(relational_db)
      attributes.collect{ |attr| attr.sql_text(relational_db, self) }.join(" ")
    end

    def to_s
      "COLUMN #{name()}: #{attributes.collect{ |attr| attr.to_s }.join(", ")}"
    end
  end

  class RowDef
    attr_reader :column_defs_to_values

    def initialize(column_def_to_value_hash)
      @column_defs_to_values = column_def_to_value_hash
    end

    def column_names_to_defs
      @column_names_to_defs ||= begin
        column_names = @column_defs_to_values.keys.collect{ |cdef| cdef.name }
        column_defs = @column_defs_to_values.keys
        Hash[column_names.zip(column_defs)]
      end
    end

    def set_column_value(column_name, value)
      column = column_names_to_defs()[column_name]
      raise "No such column '#{column_name}'" if column.nil?
      @column_defs_to_values[column] = value
    end

    def columns
      column_defs_to_values().keys
    end

    def values
      column_defs_to_values().values
    end

    def to_s
      column_defs_to_values().collect{ |key, val| "#{key}: #{val}" }.join(", ")
    end
  end

  class RowDefs
    attr_reader :row_defs
    attr_reader :column_names_to_defs

    def initialize(row_defs = [])
      @row_defs = row_defs
      @column_names_to_defs = {}
    end

    def add_column(column_def)
      @column_names_to_defs[column_def.name] = column_def
      @row_defs.each do |rowdef|
        rowdef.column_defs_to_values[column_def] = nil
      end
    end

    def add_row(column_names_to_values)
      column_defs_to_values = {}

      column_names_to_values.each do |column_name, value|
        column = column(column_name)
        raise "No column '#{column_name}'" if column.nil?
        column_defs_to_values[column] = value
      end

      @row_defs << RowDef.new(column_defs_to_values)
    end
    
    def column_defs
      column_names_to_defs().values
    end

    def column_names
      column_defs().collect{ |column_def| column_def.name }
    end

    def column(name)
      column_names_to_defs()[name]
    end
  end

  attr_accessor :resolver
  
  def initialize(db_file_path)
    @db = SQLite3::Database.new(db_file_path.to_sys)
    @db.type_translation = false
    @resolver = RelationalDBResolver.new(self)
  end
  
  def close
    if !@db.closed?
      @db.close
      @db = nil
    end
  end
  
  def retrieve(object_class, unique_id, object_specifier, attr_ref = nil, id_key_name='unique_id', absent_ok=true, object_and_id_hierarchy=nil)
    raise "Object has already been retrieved and linked (#{object_and_id_hierarchy})" unless object_and_id_hierarchy.nil? || @resolver.find(object_and_id_hierarchy).nil?

    table_name = table_name_for(object_class, object_specifier)
    columns = columns_for_table(table_name)
    db_id = unique_id
    attr_refs = attr_refs_for_object_class_config_vars(object_class)

    results = 
      if table_exists?(table_name)
        execute <<-EOS
select #{columns.collect{ |c| quote(c) }.join(", ")} from #{quote(table_name)} where #{quote(id_key_name)} = #{quoteval(db_id)}
        EOS
      else
        []
      end
    
    if results.empty?
      if absent_ok
        return nil
      else
        raise "No data returned."
      end
    end

    new_obj = 
      if object_class.respond_to?(:relational_db_initialize)
        object_class.relational_db_initialize(self, table_name, attr_ref)
      else
        object_class.allocate
      end

    if new_obj.nil?
      raise "Nil allocated object while loading class '#{object_class}'."
    end

    #    raise "#{results.length} results from select: error in db primary key def?" if result.length > 1

    restored_values = {}

    results.each do |result|
      columns.zip(result) do |column_name, value|
        value_to_set = nil
        symbol_name = nil
        value_object_class = nil
        
        if value.kind_of?(String) && value.start_with?("dbobjref://")
          uri = URI(value)
          symbol_name = column_name
          column_attr_ref = attr_refs.find { |ref| symbol_name.to_s == symbol_name }
          value_to_set = RelationalDBWeakRef.ref_if_not_resolved(column_attr_ref, new_obj, uri, self)
        elsif value.kind_of?(String) && value.start_with?("!hosted")
          # Load hosted content.
          match = /!hosted-([^!]+)$/.match(value)
          raise "Malformed 'hosted' tag for id #{unique_id}: #{key}, '#{value}'" if !match

          value_object_class = eval(match[1])
          host_class = object_class
          host_id = unique_id

          symbol_name = column_name
          column_attr_ref = attr_refs.find { |ref| symbol_name.to_s == symbol_name }
          raise "No attr ref for symbol '#{symbol_name}' in class '#{object_class}'" if column_attr_ref.nil?

          value_to_set = 
            if value_object_class.respond_to?(:relational_db_retrieve_hosted_object)
              value_object_class.relational_db_retrieve_hosted_object(self, host_class, column_attr_ref)
            else
              retrieve(value_object_class, symbol_name, host_class, column_attr_ref, 'host_symbol', false)
            end
        else
          symbol_name = column_name
          value_to_set = value
        end

        restored_values[symbol_name] ||= []
        restored_values[symbol_name] << value_to_set
      end
    end

    restored_values.each do |symbol_name, values|
      column_attr_ref = attr_refs.find { |ref| ref.symbol.to_s == symbol_name }

      if column_attr_ref.nil?
        if new_obj.relational_db_restore(symbol_name, values, attr_ref) == nil
          raise "No attribute reference for column '#{symbol_name}' and no custom restore method defined in class '#{new_obj.class}'." 
        end
      else
        # tbd: verify that attr is one of limited class of non-writable attr refs, such as unique_id
        if column_attr_ref.writable?
          raise "Too many values for attribute '#{attr_ref}'." if values.length != 1
          column_attr_ref.set(values.first, new_obj)
        end
      end
    end

    # tbd: remove type check
    if new_obj.kind_of?(Unique)
      @resolver.link(new_obj)
    end

    new_obj
  end

  def rowdefs_for_config_object(object)
    rowdefs = RowDefs.new

    row = {}

    attr_refs = attr_refs_for_object_class_config_vars(object.class)

    attr_refs.each do |attr_ref|
      if object.class.should_write_config_variable?(attr_ref.symbol.to_s, self, false)
        attr_value = attr_ref.get(object)
        
        if attr_value.respond_to?(:relational_db_store?)
          if attr_value.relational_db_store?
            rowdefs.add_column(ColumnDef.new(attr_ref.symbol.to_s))
            row[attr_ref.symbol.to_s] = attr_value.relational_db_store(self, attr_ref, object, true)
          else
            row_info = attr_value.relational_db_format(attr_ref, self)
            row_info.each do |column_def, value|
              rowdefs.add_column(column_def)
              row[column_def.name] = value
            end
          end
        else
          rowdefs.add_column(ColumnDef.new(attr_ref.symbol.to_s))
          row[attr_ref.symbol.to_s] = attr_value
        end
      end
    end

    object.class.relational_db_extra_columns.each do |column_def|
      rowdefs.add_column(column_def)
    end

    object.relational_db_extra_column_values.each do |column_name, value|
      row[column_name] = value
    end

    rowdefs.add_row(row)

    rowdefs
  end

  def store(unique_host, attr_ref=nil, replace=true)
    note { "Store host '#{unique_host.class}'." }
    _store_flat_data(rowdefs_for_config_object(unique_host), unique_host.class, attr_ref, replace, nil)
  end
  
  def _store_flat_data(rowdefs, object_class, attr_ref, replace, object_specifier)
    note { "Store '#{object_class}'." }

    if !ensure_table_for(rowdefs, object_class, object_specifier)
      log { "#{object_class}: Nothing to store." }
      return
    end

    operation = if replace
                  'insert or replace'
                else
                  'insert'
                end
    
    quoted_column_names = rowdefs.column_names.collect{ |column_name| quote(column_name.to_s) }
    value_clauses = ["?"] * quoted_column_names.length

    table_name = table_name_for(object_class, object_specifier)
      
    rowdefs.row_defs.each do |rowdef|
      binds = rowdef.values
      p '----------------------------------------------------------------------'
      p binds
      
      execute_string = <<-EOS
      #{operation} into #{quote(table_name)}
        ( #{quoted_column_names.join(',')} )  
        values ( #{value_clauses.join(',')} );
      EOS
      
      execute(execute_string, *binds)
    end
    
    table_name
  end

  def quote(value)
    '"' + value.to_s.gsub('"','""') + '"'
  end

  def quotestr(value)
    '\'' + value.to_s.gsub('\'','\'\'') + '\''
  end

  def quoteval(value)
    if value.kind_of?(Numeric)
      value.to_s
    else
      quotestr(value)
    end
  end

  def quotejoin(values, join_str=',')
    values.collect do |value|
      quote(value)
    end.join(join_str)
  end

  def execute(sql_string, *binds)
    note { sql_string + if binds.length != 0 then "Values: #{binds.join(', ')}"; else ''; end }
    @db.execute(sql_string, *binds)
  end

protected
  def attr_refs_for_object_class_config_vars(object_class)
    object_class.configVariables
  end

  def ensure_row_defs_unique_columns(row_defs)
    if row_defs.uniq.length != row_defs.length
      raise "Duplicate entries in row defs (#{row_defs - row_defs.uniq}.join(", "))"
    end
  end

  def column_def_for_object_class(row_defs, object_class)
    ensure_row_defs_unique_columns(row_defs)

    columns = row_defs.columns

    if columns.empty?
      raise "No columns can be defined for this object class. Are there persistent variables defined?"
    end

    result = ''
    
    columns.each do |column|
      if result.empty?
        result << "("
      else
        result << ","
      end
      
      result << column.sql_text(self)
    end
    
    result << ")" if !result.empty?
  end

  def row_exists_for?(unique, object_specifier)
    table_name = table_name_for(unique.class, object_specifier)
    primary_key = primary_key_column_name_for(unique.class)
    
    result = execute("select EXISTS ( #{select_query_for(unique)} ) ").first
    
    log { "Row with id '#{unique.unique_id}' exists: #{result}" }
    result != 0
  end
  
  def select_query_for(unique, object_specifier)
    table_name = table_name_for(unique.class, object_specifier)
    primary_key = primary_key_column_name_for(unique.class)
    
    "select * from #{quote(table_name)} #{where_clause_for(unique)}"
  end

  def table_constaint_for_object_class(object_class)
    primary_key = primary_key_column_for(object_class)
    "primary key "
  end

  def table_exists?(table_name)
    result = !execute("select name from sqlite_master where name=#{quote(table_name)}").empty?
    log { "Table '#{table_name}' exists: #{result}" }
    result
  end

  def table_name_for(object_class, specifier)
    if specifier
      "#{object_class.to_s}!#{specifier}"
    else
      object_class.to_s
    end
  end

  def columns_for_table(table_name)
    execute("pragma table_info(#{quote(table_name)})").collect { |e| e[1] }
  end

  def ensure_table_for(row_defs, object_class, object_specifier)
    table_name = table_name_for(object_class, object_specifier)

    if table_exists?(table_name)
      table_columns = columns_for_table(table_name)

      if table_columns.empty?
        return false
      end

      new_columns = row_defs.column_names.collect{ |sym| sym.to_s } - table_columns

      if new_columns.length != 0
        log { "Table '#{table_name}' requires schema change." }

        transaction do
          new_columns.each do |column|
            p column
            p row_defs
            p row_defs.column(column)
            p row_defs.column_names_to_defs
            execute "alter table #{quote(table_name)} add column #{row_defs.column(column).sql_text(self)}"
          end
        end
      end
    else
      log { "Creating table '#{table_name}'" }

      column_defs = row_defs.column_defs

      execute <<-EOS
      create table #{quote(table_name)} (#{column_defs.collect{ |column_def| column_def.sql_text(self) }.join(", ")})
      EOS
    end

    true
  end
  
  def transaction(&block)
    note { "Begin transaction." }
    @db.transaction &block
    note { "End transaction." }
  end

  def execute_and_return_columns(sql_string, *binds)
    note { sql_string }
    @db.execute2(sql_string, *binds)
  end

  def execute_batch(sql_string, *binds)
    note { sql_string }
    @db.execute_batch(sql_string, *binds)
  end

  def where_clause_for(unique)
    primary_key = primary_key_column_name_for(unique.class)
    "where #{quote(primary_key)}=#{unique.unique_id}"
  end 
end

RelationalDB.log = true
