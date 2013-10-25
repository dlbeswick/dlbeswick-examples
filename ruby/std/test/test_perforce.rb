require 'test/unit'
require 'perforce'

class TC_Perforce < Test::Unit::TestCase
	def test_finalizer
		Perforce.echo = false
		
		p = P4Change.new('perforce.rb test case')
		
		number = p.number
		
		p = nil
		GC.start
		GC.start
		GC.start
		GC.start
		GC.start

		pending_changes = Perforce.p4('changes -s pending')
		
		assert(
			/Change #{number}/m.match(pending_changes) == nil,
			"Finalized pending change #{number} was not reverted, Perforce status:\n" + pending_changes
		)
	end
end