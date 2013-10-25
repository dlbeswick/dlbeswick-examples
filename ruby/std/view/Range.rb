require 'std/view/View'

module Views
  class Range < View
    include AttributeReferenceView
    
    suits ::Range
    
    def generate
      super

      if attribute().writable?
        @attributeReferenceFirst = ::AttributeReference::RangeFirst.new(target(), attribute())
        @attributeReferenceFirst.viewable_dependant(self)
        @attributeReferenceLast = ::AttributeReference::RangeLast.new(target(), attribute())
        @attributeReferenceLast.viewable_dependant(self)
      else      
        @attributeReferenceFirst = ::AttributeReference::Object.new(@target.get.first)
        @attributeReferenceFirst.viewable_dependant(self)
        @attributeReferenceLast = ::AttributeReference::Object.new(@target.get.last)
        @attributeReferenceLast.viewable_dependant(self)
      end
      
      layoutAddHorizontal(View.newViewFor(@attributeReferenceFirst, self))
      layoutAddHorizontal(makeStaticText(self, '...'))
      layoutAddHorizontal(View.newViewFor(@attributeReferenceLast, self))
    end
    
    def onViewableDependencyEvent(record)
      return if record.instigator == self
      
      case record.who
        when @attributeReferenceFirst
          target().modified(self)
        when @attributeReferenceLast
          target().modified(self)
        else
          @attributeReferenceFirst.set(target().get.first)
          @attributeReferenceFirst.modified(record)
          @attributeReferenceLast.set(target().get.last)
          @attributeReferenceLast.modified(record)
      end
    end
  end
end
