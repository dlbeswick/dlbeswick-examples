require 'std/platform_gui'

if PlatformGUI.wx?
  # A list of selections that the user can choose from
  class GUISelections
    include Viewable
    
    attr_reader :message
    attr_reader :title
    attr_reader :data_or_textdata
    attr_reader :action_block
    attr_reader :default
    
    def initialize(message, data_or_textdata, default=nil, title='', &action_block)
      @message = message
      @title = title
      @data_or_textdata = data_or_textdata
      @default = default
      @action_block = action_block
    end
    
    def marshal_dump
      [@message, @title, @data_or_textdata]
    end
    
    def marshal_load
      raise "This object is not intended to be loaded from a dump."
    end
  end
  
  class ViewDataSelector < ViewBasicAttributeReference
    suits(GUISelections)
    
    def generate
      layoutAddVertical makeStaticText(self, target().message)
      
      combo = DataCombo.new(self, target().data_or_textdata, target().default)
      layoutAddVertical combo.gui
      
      layoutAddHorizontal makeButton(self, 'Ok') {
        target().action_block.call(combo.selected_data)
        container().destroy() 
      }
      
      layoutAddHorizontal makeButton(self, 'Cancel') { container().destroy() }
    end
    
    def viewable_name
      target().title
    end
    
    def self.order
      -0.9
    end
  end
else
  class DataSelector
  	# textData can be either an array of single elements or of two element arrays representing text and data
  	def initialize(app, textData, defaultSelection=nil, title="Make a selection", &onSelect)
  		@dlg = FXDialogBox.new(app, title, :width => 200, :height => 300)
  		@dlg.create
  		
  		@onSelect = onSelect
  		
  		@list = FXList.new(@dlg) { |ctrl|
  			ctrl.layoutHints = LAYOUT_FILL_X
  			ctrl.numVisible = 15
  			textData.each do |e|
  				if e.respond_to?(:[])
  					ctrl.appendItem(e[0].to_s, nil, e[1])
  				else
  					ctrl.appendItem(e.to_s, nil, e)
  				end
  			end
  			
  			ctrl.connect(SEL_DOUBLECLICKED) { select }
  			
  			ctrl.create
  
  			if ctrl.numItems > 0
  				if defaultSelection
  					ctrl.selectItem(ctrl.findItemByData(defaultSelection))
  				else
  					ctrl.selectItem(0, true)
  				end
  			end
  		}
  		
  		FXButton.new(@dlg, "Ok") { |ctrl|
  			ctrl.layoutHints = LAYOUT_SIDE_LEFT
  			ctrl.connect(SEL_COMMAND) { select }
  			ctrl.create
  		}
  
  		@dlg.recalc
  		@dlg.show(PLACEMENT_CURSOR)
  	end
  	
  	def select
  		if @list.currentItem >= 0
  			item = @list.getItem(@list.currentItem)
  			if @onSelect.arity > 1
  				@onSelect.call(item.text, item.data)
  			else
  				@onSelect.call(item.data)
  			end
  		end
  		
  		@dlg.close
  	end
  end
end
