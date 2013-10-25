require 'std/ConfigYAML'
require 'std/SymbolDefaults'

module Mapping
	class Base
		include ConfigYAML
		
		persistent :srcMin
		persistent :srcMax
		persistent :dstMin
		persistent :dstMax
		persistent :clamp
	end

	class AllToOne
		persistent :value
	end

	class Gate
		persistent :srcCutoff
	end

	class Power
		persistent :power
	end

	class Composite
		persistent :linkSourceRange
		persistent :linkDestinationRange
		persistent :mappings
	end

	class Sequence
    include SymbolDefaults

    persistent :normalizedSrc
		persistent :normalizedDest

		default :normalizedSrc, true
		default :normalizedDest, true
	end

	class Threshold
		persistent :thresholds
	end

	class DataPoints
		persistent :dataPoints
		persistent :interpolationMapping
	end
end
