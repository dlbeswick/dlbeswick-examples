require 'std/view/labeled_field'

# Views the choices for an attribute that the developer enumerated using the "symbol defaults" system.
# tbd: should be renamed to reflect this
class ViewComboChoice < ViewBasicAttributeReference
  def self.suits_attribute?(attribute)
    super && 
      attribute.viewable_kind_of?(AttributeReference::AttributeSymbol) &&
      attribute.host.viewable_kind_of?(SymbolDefaults) && 
      attribute.host.default_choices_for_symbol?(attribute.symbol)
  end
  
  def self.suits_target?(target, target_class)
    true
  end
  
  def self.order
    -0.8
  end
  
protected
  def make_controls
    choices = attribute().host.default_choices_for_symbol(attribute.symbol)
    combo = DataCombo.new(self, choices, target()) do |data|
      if data
        attribute().set(data)
        attribute().modified(self)
      end
    end
    
    combo.editable = attribute().writable?
    
    layoutAddVertical(combo.gui)
  end
end

class ViewComboChoiceOfObjectReferences < ViewComboChoice
  def self.suits_attribute?(attribute)
    if super
      choices = attribute.host.default_choices_for_symbol(attribute.symbol)
      !choices.empty? && choices.find { |x| Views.basic_type?(x) } == nil
    else
      false
    end
  end
end
