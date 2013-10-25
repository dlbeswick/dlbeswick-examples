require 'std/view/View'

class ViewInferredAttribute < ViewBasicAttributeReference
  attr_accessor :edit_mode
  
  def self.order
    -0.7
  end
  
  def self.suits_attribute?(attribute)
    super && attribute.viewable_kind_of?(AttributeReference::AttributeSymbol) && 
      attribute.host.class.respond_to?(:inferred_attribute?) &&
      attribute.host.class.inferred_attribute?(attribute.symbol)
  end
  
  def self.suits_target?(target, target_class)
    true
  end
  
protected
  def button_text(should_return_inferred_text)
    if should_return_inferred_text
      'Edit'
    else
      'Default'
    end
  end
  
  def data_is_inferred?
    attribute().host.class.inferred_value?(attribute().symbol, attribute().host)
  end
  
  def generateExecute
    if @view
      @view.view_destroy
      @view = nil
      #GC.start
    end
    
    attribute_for_view = if data_is_inferred?
      AttributeReference::Reader.new(attribute().symbol, attribute().host)
    else
      attribute()
    end

    if !@button
      @button = makeButton(self, 'Edit') do
        if data_is_inferred?
          # data is inferred, switch to editable mode

          # a model value should always be available, because the inferred value 
          # is one of the model values.
          value_to_initiate_edit = attribute().host.model_value(attribute().symbol)
          attribute().set(value_to_initiate_edit)
        else
          # data is edited, switch to inferred mode
          
          attribute().host.class.symbol_reset_to_inferred_value(attribute().symbol, attribute().host)
        end
        
        generate()
        
        attribute().modified(self)
        self.edit_mode = !data_is_inferred?
      end
    
      layoutAddHorizontal(@button)
    end
    
    @button.label = button_text(data_is_inferred?)
    
    @view = View.newViewFor(attribute_for_view, self, View.subclasses - ViewInferredAttribute.subclasses)
    layoutAddHorizontal(@view)
  end
end
