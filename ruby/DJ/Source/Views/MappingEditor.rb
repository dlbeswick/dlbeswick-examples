require 'std/DataCombo'
require 'std/view/GraphView'
require 'std/view/View'

class MappingEditorBase < ViewBasicAttributeReference
  include Views::AttributeReferenceView
	include PlatformGUISpecific

	suits Mapping::Base	
  
  def self.order
    -1
  end
end

class MappingEditorWx < MappingEditorBase
	include PlatformGUISpecific

	def make_controls
		super
		
		horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
		self.sizer.add(horz, 1, Wx::GROW)
		
		@graphView = GraphView.new(attribute(), self)
		horz.add(@graphView, 1, Wx::GROW)

    Wx::ScrolledWindow.new(self) do |scrolled|
    	scrolled.set_scroll_rate(1, 1)
    	scrolled.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
    	
      @attributesView = View.new_for(attribute(), scrolled, ViewAllAttributesClassSelector.subclasses)
			
			scrolled.sizer.add(@attributesView, 1, Wx::GROW)
			
			horz.add(scrolled, 1, Wx::GROW)
		end
	end
end

class MappingEditorFox < MappingEditorBase
	include PlatformGUISpecific

	def make_controls
		super
		
			FXHorizontalFrame.new(dlg, LAYOUT_FILL) do |packer|
				packer.create
				
				FXSpring.new(packer, LAYOUT_FILL, 60, 0) do |spring|
					spring.create
					
					DataCombo.new(spring, Mapping::Base.concreteSubclasses, mapping.class) do |c|
						newMapping = c.new
						newMapping.srcMin = mapping.srcMin
						newMapping.srcMax = mapping.srcMax
						newMapping.dstMin = mapping.dstMin
						newMapping.dstMax = mapping.dstMax
						mapping = newMapping
						observe(mapping)
						
						onChange.call(mapping)
					end

					@graphView = GraphViewMapping.new(spring, mapping)
					@graphView.layoutHints = LAYOUT_FILL
					@graphView.create
				end
				
				FXSpring.new(packer, LAYOUT_FILL, 40, 0) do |spring|
					spring.create
					
#					attrPacker = FXGroupBox.new(spring, 'Mapping Properties', FRAME_RAISED)
#					attrPacker.layoutHints = LAYOUT_FILL
#					attrPacker.create
					
					FXScrollWindow.new(spring) do |scroll|
						scroll.layoutHints = LAYOUT_FILL
						scroll.create
						
						@attributesView = Views::AllAttributes.new(nil, nil, scroll)
						class << @attributesView
							def generate
								viewMapping = { AttributeReference::Base => Views.customClassSelector(Mapping::Base) }
								View.beginViewMapping(viewMapping) do
									super
								end
							end
						end
						
						@attributesView.layoutHints = LAYOUT_FILL
					end
				end
			end
	end
end
