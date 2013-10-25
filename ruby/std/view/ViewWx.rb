require 'std/view/bordered'

module Views
	class Hash < HashCommon
		def makeHashItem(keyView, valueView)
			horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
			self.sizer.add(horz)
			
			horz.add(keyView)
			
			Wx::StaticText.new(self, -1, '=>') do |c|
				horz.add(c)
				@views << c
			end
			
			horz.add(valueView)
		end
		
		def makeTargetDescription(description)
			Wx::StaticText.new(self, -1, description) do |c|
				self.sizer.add(c)
			end
		end
	end

	class ObjectReference < ObjectReferenceCommon
		def make_controls
			self.sizer.add(Wx::StaticText.new(self, -1, self.labelText))
		end
	end
end
