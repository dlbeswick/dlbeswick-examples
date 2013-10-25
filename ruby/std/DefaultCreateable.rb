module DefaultCreateable
	include IncludableClassMethods
	
	module ClassMethods
		def createDefault
			new()
		end
	end
end

module DefaultCreateableOwned
	include IncludableClassMethods

	module ClassMethods
		def createDefaultWithOwner(owner)
			self.new(owner)
		end
	end
end