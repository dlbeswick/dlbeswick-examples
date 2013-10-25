raise "Is this module deprecated in favour of SymbolDefaults?"
require 'IncludableClassMethods'

module DefaultSymbolTypes
	include IncludableClassMethods

	module ClassMethods
		def setDefaultTypeForSymbol(symbol, type)
			@defaultSymbolTypes ||= {}
			@defaultSymbolTypes[symbol] = type
		end

		def defaultTypeForSymbol(symbol)
			self.ancestors.each do |a|
				type = @defaultSymbolTypes[symbol] if @defaultSymbolTypes
				
				if type
					return type 
				else
					if a != self && a.respond_to?(:defaultTypeForSymbol)
						type = a.defaultTypeForSymbol(symbol)

						if type
							return type 
						end
					end
				end
			end

			nil
		end

		alias_method :default_type, :setDefaultTypeForSymbol
	end

	def defaultTypeNoProc(object)
		if object.class < Integer
			0
		else if object.class < Numeric
			0.0
		else if object.class < String
			''
		end
	end

	def defaultTypeForSymbol(symbol)
		self.class.ancestors.each do |a|
			if a.respond_to?(:defaultTypeForSymbol)
				return a.defaultTypeForSymbol(symbol)
			end
		end
		
		defaultTypeNoProc(get_instance_variable(symbol))
	end
end

class Object
	include DefaultSymbolTypes
end
