require 'std/SymbolDefaults'

module Mapping
	class DataPoints < Base
		include SymbolDefaults
		
		default :dataPoints, [[0.0, 1.0]]
	end
end
