require 'std/view/labeled_field'
require 'std/builtin_extend'

class ViewArrayCombo < ViewBordered
  suits Array
  attr_reader :combo
  
  def selected
    target()[combo().selected_data]
  end  
  
  def self.order
    -2
  end
  
protected
  def make_controls
    super
    
    data_array = target().collect_with_index do |e, i|
      ["(#{e.catalogue_id}) #{e.artist} - #{e.title}", i]
    end
    
    @combo = DataCombo.new(self, data_array, 0) do |data|
      view_index(data)
    end
    layoutAddVertical(@combo.gui)

    view_index(0) if !target().empty?
  end

  def view_index(idx)
    guiUpdate do
      @content.view_destroy if @content
      @content = nil
      #GC.start
      @content = View.newViewFor(AttributeReference::ArrayElement.new(idx, target()), self)
      layoutAddVertical(@content)
    end
  end
end
