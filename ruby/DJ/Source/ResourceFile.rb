require './Source/Resource'

module Resource
	class File < Base
		attr_reader :path
		persistent :path
		
		def initialize(path)
			raise Resource::Exception, "File resource #{path} does not exist." if !path.exists?
			@path = path.realpath
		end

		def id
			@path.to_s.downcase
		end
	end
end

