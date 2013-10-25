require 'std/algorithm'
require 'std/raise'
require 'std/method_chain'
require 'std/module_logging'
require 'std/path'

# If included in a class, a method 'subclasses' becomes available that returns a list of subclasses of that class.
# If included in a module, those classes that include that module are available via a call to 'subclasses' on the module.
# Other modules that included the original module are not included.
#
# The 'subclasses' function can return any subset of the subclass hierarchy via the 'kindof' parameter.
# Returning this subset can be made more efficient by re-including EnumerableSubclasses at the head of the
# hierarchy subset.
#
# Those classes where 'EnumerableSubclasses' have been included are called 'node classes.' 
#
# The following class methods are made available to subclasses:
# abstract: The class is marked as a 'abstract,' but construction is not prevented. By default, the subclass 
#   is not enumerated by the subclasses method. See also the 'Abstract' module.
# unenumerable: The class is never returned by any variation of the 'subclasses' method. 
module EnumerableSubclasses
  include MethodChain
  include ModuleLogging
  include Raise
  
	module InheritableClassMethods
	  if !respond_to?(:abstract?)
  		def abstract?
  			@abstract == true
  		end
	  end
		
		def enumerable?
			@unenumerable != true
		end

		def enumerablesubclasses_node?
			@subclasses != nil
		end

    # REMINDER: call this before including PlatformSpecific as instance variables are copied then
    # tbd: find a better solution
		# tbd 2: does this comment still apply? Is it solved by the Abstract module?
		# no, that doesn't solve it
		def abstract
			@abstract = true
		end
		
		def unenumerable
			@unenumerable = true
		end

    def each(kindof = self, &block)
      subclasses(kindof).each do |c|
        yield c
      end
    end
    
    # returns concrete and abstract subclasses
    def all_subclasses(kindof = self)
      first_ancestor_subclasses().dup.delete_if do |c|
        !c.enumerable? || !(c <= kindof) 
      end
    end
    
    def first_node_module
      Algorithm.each_ancestor_kindof(self, EnumerableSubclasses, true) do |c|
        return c if c.enumerablesubclasses_node?
      end
      
      EnumerableSubclasses.raise "Couldn't find a node for class #{self}."
    end
    
    def first_node_class
      Algorithm.each_ancestor_kindof(self, EnumerableSubclasses, true) do |c|
        return c if c.enumerablesubclasses_node? && c.kind_of?(Class)
      end
      
      EnumerableSubclasses.raise "Couldn't find a node for class #{self}."
    end

    # only returns concrete subclasses
    def subclasses(kindof = self)
      all_subclasses(kindof).delete_if { |c| c.abstract? }
    end
  
  protected
    def first_ancestor_subclasses
      if !@first_ancestor_subclasses
        each_ancestor_subclasses() do |list|
          @first_ancestor_subclasses = list
          break
        end
      end
      
      @first_ancestor_subclasses
    end

    def each_ancestor_subclasses(&block)
      Algorithm.each_ancestor(self, true) do |a|
        if a.respond_to?(:subclassesListReader)
          list = a.subclassesListReader
          if list
            yield list 
          end
        end
      end
    end
	end
	
	module NodeClassMethods
    def subclassesListReader
      @subclasses
    end
    
    def enumerablesubclasses_add_descendant(descendant)
      @subclasses << descendant
    end
	end

	# Notes on this function:
	# Defining methods in the 'singleton' context of a class is equivalent to defining class methods on that class.
	# Firstly, the singleton context must be used so that alias_method chaining will work on class methods.
	# There is no other way to chain a class method.
	# Secondly, an 'eval' must be used as opposed to a singleton block (class << self) because "node_class" must
	# be accessible in order to call 'enumerablesubclasses_add_descendant' on the correct node class.
	def self.propagate_to(node_class, receiver)
    receiver.extend(EnumerableSubclasses::InheritableClassMethods)
    
    # the second check is to fix multiple inclusion (see test case of MultiBaseModuleIndirectIncludeClass.)
    # the receiver has already been added to a base module via the base modules inclusion in a second included
    # module.
    # I don't think there's a better way to solve this problem, unless the modules know about each other beforehand.
    if node_class != receiver && !node_class.subclasses.include?(receiver)
      node_class.enumerablesubclasses_add_descendant(receiver)
    end
    
    receiver.instance_eval { include MethodChain }
       
    if receiver.kind_of?(Class)
      receiver.module_chain "EnumerableSubclasses_#{node_class}", :inherited, node_class do |super_info, c, node_class|
        MethodChain.super(self, super_info, c)
        node_class.enumerablesubclasses_add_descendant(c)
      end
    else
      receiver.module_chain "EnumerableSubclasses_#{node_class}", :included, node_class do |super_info, receiver2, node_class|
        MethodChain.super(self, super_info, receiver2)
        EnumerableSubclasses.propagate_to(node_class, receiver2)
      end
    end
	end
	
	module_chain EnumerableSubclasses, :included do |super_info, receiver|
    MethodChain.super(self, super_info, receiver)
    
    if receiver.respond_to?(:enumerablesubclasses_node?) && receiver.enumerablesubclasses_node?
      # nothing
    else
      receiver.instance_variable_set(:@subclasses, [])
      receiver.extend(EnumerableSubclasses::NodeClassMethods)

      EnumerableSubclasses.propagate_to(receiver, receiver)
    end
	end

  # Require all the files with the given path prefix or matching the wildcard glob.
  # If include_system is true, then ruby system library paths are enumerated and the
  # basename of the input path is appended. The first found system path has its contents
  # required, or those files matching the wildspec are required.
  def self.require(prefix_or_wildspec, include_system=true)
    searchpaths = []
      
    search_prefix_path = if Path.wild?(prefix_or_wildspec)
      Path.new(prefix_or_wildspec)
    else
      Path.to_directory(prefix_or_wildspec).with_file_replace('*.rb')
    end

    log { "Requiring #{search_prefix_path}" }
      
    if include_system
      searchpaths += $:.collect do |string|
        Path.new(string) + search_prefix_path
      end
    end

    searchpaths = [search_prefix_path] + searchpaths
    
    searchpaths.each do |searchpath|
      if searchpath.unwild.exists?
        note { "search path: #{searchpath.absolute}" }
        searchpath.ls do |path|
          target = path.no_extension.literal
          log { "#{target}" }
          Kernel.require(target)
        end
        break
      end
    end
  end
end
