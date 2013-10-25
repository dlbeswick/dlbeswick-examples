# A gui-platform-independent way to instantiate a combo box.
# The user can pass either an array of text strings, or an array of text/data tuples.  
module DataComboBase
	attr_accessor :onSelect
	
	def datacombobase_initialize(parent, textDataArray, default=nil, &onSelect)
		self.data = textDataArray
    @onSelect = onSelect
		select(default)
  end

  def gui
  end

  def clear
    PlatformGUI.specific
  end

	def data=(textDataArray)
		datacombobase_clearCombo

		textDataArray.each do |e|
			if e.kind_of?(Array)
				datacombobase_addItem(e[0].to_s, e[1])
			else
				datacombobase_addItem(e.to_s, e)
			end
		end
	end
	
	def editable=(b)
    PlatformGUI.specific
	end
	
	def select(data)
		PlatformGUI.specific
    false
	end
	
  def select_by_text(text)
    PlatformGUI.specific
    false
  end
  
	def selected_data
    datacombobase_currentData()
	end
	
protected
	def datacombobase_onItemSelected
		return if !@onSelect
		 
		if @onSelect.arity == 2
			@onSelect.call(datacombobase_currentText, datacombobase_currentData)
		else
			@onSelect.call(datacombobase_currentData)
		end
	end
	
	def datacombobase_addItem(text, data)
		PlatformGUI.specific
	end
	
	def datacombobase_currentText
		PlatformGUI.specific
	end
	
	def datacombobase_currentData
		PlatformGUI.specific
	end
	
	def datacombobase_clearCombo
		PlatformGUI.specific
	end
end

class DataComboWx
	include PlatformGUISpecific
	include DataComboBase

  attr_reader :gui

	def initialize(parent, textDataArray, default=nil, &onSelect)
		@gui = Wx::ComboBox.new(parent, -1, :style => Wx::CB_DROPDOWN | Wx::CB_READONLY) {}
		
		datacombobase_initialize(parent, textDataArray, default, &onSelect)

		parent.evt_combobox(@gui) do
			datacombobase_onItemSelected
		end
	end

  def clear
    @gui.clear
  end

	def editable=(b)
    @gui.enable(b)
	end
	
	def select(data)
		@gui.each do |i|
			if @gui.item_data(i).data == data
				@gui.selection = i
				return true
			end
	  end
  
    false
	end

  def select_by_text(text)
    item = @gui.find_string(text)
    if item != nil
      @gui.selection = item
      true
    else
      false 
    end
  end
  
protected
  # tbdwx: this class is necessary because adding SWIG objects as data to a combo box results in a "can't
  # convert nil to string" exception. 
  class Data
    attr_reader :data
    
    def initialize(data)
      @data = data
    end
  end

	def datacombobase_addItem(text, data)
		@gui.append(text, Data.new(data))
	end
	
	def datacombobase_currentText
		@gui.string_selection
	end
	
	def datacombobase_currentData
		@gui.item_data(@gui.selection).data
	end
	
	def datacombobase_clearCombo
		@gui.clear
	end
end

class DataComboFox
	include PlatformGUISpecific
	include DataComboBase

	def initialize(parent, textDataArray, default=nil, &onSelect)
		makeCombo(parent)
		datacombobase_initialize(parent, textDataArray, default, &onSelect)
	end

	def select(data)
		idx = @combo.findItemByData(data)
		@combo.setCurrentItem(idx)
		
		if idx == -1
			@combo.setText(data.to_s)
			@combo.setTextColor(FXColor::Red)
		else
			@combo.setTextColor(FXColor::Black)
		end
	end

protected
	def datacombobase_addItem(text, data)
		@combo.appendItem(text, data)
	end
	
	def makeCombo
		@combo = FXComboBox.new(parent, 20, nil, 0, COMBOBOX_STATIC) { |ctrl|
			ctrl.numVisible = 15
			
			ctrl.connect(SEL_COMMAND) { self.datacombobase_onItemSelected if @combo.currentItem >= 0 }
			
			ctrl.create

			ctrl.setCurrentItem(ctrl.findItemByData(default), false) if ctrl.numItems > 0
		}
	end
	
	def datacombobase_clearCombo
		@combo.clearItems
	end

	def datacombobase_currentText
		@combo.getItem(@combo.currentItem)
	end
	
	def datacombobase_currentData
		@combo.getItemData(@combo.currentItem)
	end
end
