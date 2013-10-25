require 'std/ConfigYAML'
require 'thread'
require 'drb'

module Resource
  class Exception < RuntimeError
  end
  
  class Base
    include ConfigYAML
    include Unique
	
    use_id :id
    use_id_context :resource_cache, :resources
    
    class Cache
      include ConfigYAML
      include UniqueHost

      unique_id_registry Resource::Base, :resources
      
      def initialize
      end

      def self.exists?
        config_exists? && !$quick_musiccollection
      end
      
      def self.configElementName
        "ResourceCache"
      end
    end

    @@cache = nil
    @@cacheDirty = false
    @@saveMutex = Mutex.new
    @@getMutex = Mutex.new

    def self.get(id)
      @@getMutex.synchronize do
        if Resource::Base._cache().resources.has?(id)
          Resource::Base._cache().resources[id]
        else
          puts "Resource #{id} creating..."
          nil
        end
      end				
    end
    
    def self.cache(resource)
      @@saveMutex.synchronize do
        Resource::Base._cache().resources.add(resource)
        @@cacheDirty = true
      end
    end

    def self.saveCache
      if @@cacheDirty
        puts "Writing resource cache..."
        if $quick_musiccollection
          puts "(not saving, quick music collection.)"
        elsif Resource::Base._cache().resources.empty?
          puts "(not saving, cache is empty.)"
        else
          @@saveMutex.synchronize do
            Resource::Base._cache().save
            puts "Resource cache saved."
            @@cacheDirty = false
          end
        end
      else
        puts "Resource cache up-to-date."
      end
    end
    
    def id
      raise 'Must provide an id'
    end
	
    def description
      'Resource'
    end
    
    def resource_cache
      Resource::Base._cache()
    end

	protected
    def self._cache
      if !@@cache
        if Cache.exists?
          begin
            puts "Loading resource cache..."
            @@cache = Cache.load
            puts "Resource cache loaded (#{@@cache.resources.length} entries.)"
          rescue Exception=>e
            puts "Failure loading resource cache:"
            puts e

            #@@cache = Cache.new
            raise
          end
        else
          puts "Resource cache not found, creating..."
          puts "(quick music collection...)" if $quick_musiccollection 
          @@cache = Cache.new
        end
        
        # tbd: fix, find out why UniqueHost overrides this method via UniqueIDConfig where it should already have been changed in the Cache class definition.
        @@cache.class.module_eval do
          def configElementName
            'ResourceCache'
          end
        end
      end
      
      @@cache
    end
  end
end
