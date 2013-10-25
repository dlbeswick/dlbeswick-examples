if false
module Views
	class BasicAttributeReference < BasicAttributeReferenceCommon
    include PlatformGUISpecific
    
		def generate
			if !@frame
				@frame = FXHorizontalFrame.new(self) { |c|
					c.create
	
					@label = FXLabel.new(c, '') { |l|
						l.create
					}
	
					if Views.type_mutable?(@target)
						@text = FXTextField.new(c, 1) { |t|
							t.create
							
							t.connect(SEL_COMMAND) do
								t.selectAll
								commit(t.text)
							end
		
							t.connect(SEL_LEFTBUTTONPRESS) do
								t.selectAll
							end
						}
					else
						FXTextLabel.new(c, 'nil')
					end
				}
			end
			
			@label.setText(@target.description)
			
			if @text
				@text.setNumColumns(self.targetValue.to_s.size)
				@text.setText(self.targetValue.to_s)
			end
		end
	end

	class ArrayElement < ArrayElementCommon
		def generate
			super

			FXHorizontalFrame.new(self) do |packer|
				packer.create
				packer.padBottom = 0
				packer.padTop = 0
				packer.vSpacing = 0
				
				FXGroupBox.new(packer, "[#{view.target.idx}]", FRAME_RAISED|LAYOUT_FILL_X) do |ctrl|
					ctrl.padBottom = 0
					ctrl.padTop = 0
					ctrl.vSpacing = 0
					ctrl.create
					view.reparent(ctrl, nil)
				end
				
				if arrayView.newObjectClass(view.target.idx)
					FXButton.new(packer, '+') do |ctrl|
						ctrl.create
						ctrl.layoutHints = LAYOUT_RIGHT
		
						ctrl.connect(SEL_COMMAND) do
							@arrayView.addNewEntry(view.target.idx)
						end
					end
				end
	
				FXButton.new(packer, 'X') do |ctrl|
					ctrl.create
					ctrl.layoutHints = LAYOUT_RIGHT
					
					ctrl.connect(SEL_COMMAND) do
						@arrayView.removeEntry(view.target.idx)
					end
				end
			end
		end
	end
		
	class Array < ArrayCommon
		def clear(all=false)
			super
			if @frame
				@frame.each_child do |c|
					self.removeChild(c)
				end
			end
		end
		
		def makeGUI
			if !@frame
				horz = FXHorizontalFrame.new(self)
				horz.create
				
				FXLabel.new(horz, @target.description) do |ctrl|
					ctrl.create
				end
	
				if self.newObjectClass(0, true)
					FXButton.new(horz, '+') do |ctrl|
						ctrl.create
						ctrl.connect(SEL_COMMAND) do
							addNewEntry(0, true)
						end
					end
				else
					FXLabel.new(self, '(array of indeterminate type)') do |ctrl|
						ctrl.create
					end
				end

				@frame = FXVerticalFrame.new(self)
				@frame.layoutHints = LAYOUT_FIX_X
				@frame.padBottom = 0
				@frame.padTop = 0
				@frame.vSpacing = 0
				@frame.create
			end
		end
		
		def elementGUIParent
			@frame
		end
	end
	
	class Hash < HashCommon
		def generate
			clear
			
			hashObject = @target.get
		
			horz = FXHorizontalFrame.new(self)
			horz.create
			
			FXLabel.new(horz, @target.description) do |ctrl|
				ctrl.create
			end

			@frame = FXVerticalFrame.new(self)
			@frame.layoutHints = LAYOUT_FIX_X
			@frame.create
			
			hashObject.each do |k, v|
				FXHorizontalFrame.new(@frame) do |packer|
					packer.setPadTop(0)
					packer.setPadBottom(0)
					packer.setPadLeft(0)
					packer.setPadRight(0)
					packer.create
					
					addView(View.newViewFor(k, packer))
					
					FXLabel.new(packer, '=>') do |ctrl|
						ctrl.create
					end
					
					addView(View.newViewFor(AttributeReference::HashValueElement.new(k, @target.symbol, @target.host, @target), packer))

					FXButton.new(packer, 'X') do |ctrl|
						ctrl.create
						ctrl.layoutHints = LAYOUT_RIGHT
						
						ctrl.connect(SEL_COMMAND) do
							hashObject.delete(k)
							@target.notifyViews
						end
					end
				end
			end
		end
	end

	class ObjectReference < ObjectReferenceCommon
		def generate
			if !@text
				@text = FXLabel.new(self, '')
				@text.create
			end
			
			@text.text = self.labelText
		end
	end
end

class AllAttributesViewFox < AllAttributesViewCommon
  include PlatformGUISpecific
  
  def generate
    if viewableAttributes.empty?
      hide
      return 
    end
    
    self.padBottom = 0
    self.padTop = 0
    self.vSpacing = 0

    show
  
    if !@packer
      @packer = FXGroupBox.new(self, 'Properties', FRAME_RAISED|LAYOUT_FILL)
      @packer.padBottom = 0
      @packer.padTop = 0
      @packer.vSpacing = 0
    end

    @packer.create
    @packer.reparent(self, nil)

    super
  end
end
end
