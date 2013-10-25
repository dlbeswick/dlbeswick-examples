require 'std/view/array'
require 'std/view/bordered'

module CommonArrayView
  def insert(idx)
    target().insert([target().length, idx].min, attribute().host.defaultObjForSymbol(attribute().symbol)[0])
    target().modified(self)
  end
  
  def can_add?
    attribute().writable? && attribute().host.respond_to?(:defaultObjForSymbol?) && attribute().host.defaultObjForSymbol?(attribute().symbol)
  end

  def can_remove?
    attribute().writable?
  end
end

# Displays an enumerable object as 
class ViewArrayRecords < ViewBordered
  include CommonArrayView
  
  suits(::Array)

  attr_reader :index
    
  def make_controls
    super
    
    ctrl = makeButton(self, '|<', true) do
      view_index(0) if !target().empty?
    end
    layoutAddHorizontal(ctrl)
    
    ctrl = makeButton(self, '<', true) do
      view_index(@index - 1) if !target().empty? && @index > 0
    end
    layoutAddHorizontal(ctrl)
    
    if can_remove?
      ctrl = makeButton(self, 'X', true) do
        target().delete_at(@index)
        @index = [@index, target().length - 1].min
        view_index(@index) if !target().empty?
      end
      layoutAddHorizontal(ctrl)
    end
    
    @field_idx = makeText(self, '0', 3) do |text|
      idx = Math.clamp(text.to_i - 1, 0, target().length - 1) 
      view_index(idx)
    end
    layoutAddHorizontal(@field_idx, 15)
    
    @label_length = makeStaticText(self, '000')
    layoutAddHorizontal(@label_length, 5)
    updateLabelLength()
    
    if can_add?
      ctrl = makeButton(self, '+', true) do
        insert(@index)
        view_index([@index+1,target().length-1].min)
      end
      layoutAddHorizontal(ctrl, 15)
    end
    
    ctrl = makeButton(self, '>', true) do
      view_index(@index + 1) if !target().empty? && @index < target().length - 1
    end
    layoutAddHorizontal(ctrl, if can_add?; 0; else 15; end)
    
    ctrl = makeButton(self, '>|', true) do
      view_index(target().length - 1) if !target().empty? 
    end
    layoutAddHorizontal(ctrl)
    
    if can_add?
      ctrl = makeButton(self, 'A', true) do
        insert(target().length)
        view_index(target().length - 1)
      end
      layoutAddHorizontal(ctrl)
    end
  
    @index = 0
  end
  
  def view_index(idx)
    @index = idx
    generate
  end
  
  def self.order
    -1
  end
  
protected
  def generateExecute
    guiUpdate do
      @content.view_destroy if @content
      @content = nil
      #GC.start
      
      if !target().empty?
        @content = View.newViewFor(AttributeReference::ArrayElement.new(@index, target()), self)
        layoutAddVertical(@content)
        updateFieldIdx(@index)
        updateLabelLength
      end
    end
  end

  def updateFieldIdx(idx)
    @field_idx.value = (idx + 1).to_s
  end
  
  def updateLabelLength
    @label_length.label = "of #{target().length}"
  end
end
