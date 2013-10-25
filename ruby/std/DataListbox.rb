module DataListboxBase
  attr_accessor :on_select
  
  def datalistboxbase_initialize(parent, text_data_array=[], default=nil, &on_select)
    self.data = text_data_array
    @on_select = on_select
    select(default) if not default.nil?
  end

  def gui
  end

  def data=(textDataArray)
    datalistboxbase_clear

    textDataArray.each do |e|
      if e.kind_of?(Array)
        datalistboxbase_add_item(e[0].to_s, e[1])
      else
        datalistboxbase_add_item(e.to_s, e)
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
  
  def selected_data
    datalistboxbase_current_data()
  end
  
protected
  def datalistboxbase_on_item_selected
    if @on_select.arity == 2
      @on_select.call(datalistboxbase_current_text, datalistboxbase_current_data)
    else
      @on_select.call(datalistboxbase_current_data)
    end
  end
  
  def datalistboxbase_add_item(text, data)
    PlatformGUI.specific
  end
  
  def datalistboxbase_current_text
    PlatformGUI.specific
  end
  
  def datalistboxbase_current_data
    PlatformGUI.specific
  end
  
  def datalistboxbase_clear
    PlatformGUI.specific
  end
end

class DataListboxWx
  include PlatformGUISpecific
  include DataListboxBase

  attr_reader :gui

  def initialize(parent, text_data_array=[], default=nil, &on_select)
    @gui = Wx::ListBox.new(parent, -1, :style => Wx::LB_SINGLE | Wx::LB_SINGLE) {}

    datalistboxbase_initialize(parent, text_data_array, default, &on_select)

    parent.evt_listbox(@gui) do
      datalistboxbase_on_item_selected
    end
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
  
protected
  # tbdwx: this class is necessary because adding SWIG objects as data to a combo box results in a "can't
  # convert nil to string" exception. 
  class Data
    attr_reader :data
    
    def initialize(data)
      @data = data
    end
  end
  
  def datalistboxbase_add_item(text, data)
    data_obj = Data.new(data)
    @gui.append(text)
    @gui.set_item_data(@gui.count()-1, data_obj)
  end
  
  def datalistboxbase_current_text
    @gui.string_selection()
  end
  
  def datalistboxbase_current_data
    if @gui.selection() != Wx::NOT_FOUND
      @gui.item_data(@gui.selection()).data
    else
      nil
    end
  end
  
  def datalistboxbase_clear
    @gui.clear()
  end
end
