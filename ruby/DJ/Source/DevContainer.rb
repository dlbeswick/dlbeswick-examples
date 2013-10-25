require 'std/EnumerableInstances'

# A container for UI controls that is shown and hidden according to the application's 'dev' setting
class DevContainer < Wx::BoxSizer
	include EnumerableInstances
	
	def initialize(parent)
		super(parent)
		updateFromApp
	end
	
	def updateFromApp
		if $djApp.dev
			show
		else
			hide
		end
		
		parent.layout
	end
end