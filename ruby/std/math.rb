module Math
	def Math.min(a, b)
		if a < b then return a else return b end
	end

	def Math.max(a, b)
		if a > b then return a else return b end
	end
	
	def Math.clamp(a, min, max)
		return Math.max(Math.min(a, max), min)
	end
end