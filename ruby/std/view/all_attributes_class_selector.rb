require 'std/view/View'
require 'std/DataCombo'

# A view of all attributes with a combo that can be used to change the type of the object.
# The object cannot be changed unless the target is a settable reference to another object,
# and so this class is only be suitable for array, hash, and symbol references.  
class ViewAllAttributesClassSelector < ViewAllAttributesReference
  def initialize(target, parentWindow)
    super(target, parentWindow)
  end
  
  def viewForSymbol(symbol)
    View.viewAttribute(symbol, target(), self, attribute())
  end

  def parentAttributeForSymbols
    attribute()
  end
  
  def makeSelector
    class_selections = if attribute().respond_to?(:symbol)
      self.class.classSelections(attribute().host, attribute().symbol, target())
    else
      self.class.classSelections(attribute().host, nil, target())
    end
  
    if class_selections.length > 1
      @classSelect = DataCombo.new(self, [])
      
      @classSelect.onSelect = proc do |cls|
        new_object = createDefaultObject(cls)
        
        begin
          attribute().set(new_object)
        ensure
          View.release_authoritive(new_object)
        end
        
        attribute().modified(self)
        generate()
        modified(self)
      end
      
      layoutAddVertical(@classSelect.gui)
      
      @classSelect.data = class_selections
      @classSelect.select(target().viewable_class)
    end
  end
  
  def make_controls
    makeSelector
    
    super
  end

  def self.order
    -0.9
  end
  
  def self.suits_attribute?(attribute)
    if (attribute.viewable_kind_of?(::AttributeReference::AttributeSymbol) && 
        attribute.host.class.respond_to?(:attribute_hosted?))
      attribute.host.class.attribute_hosted?(attribute.symbol)
    else
      attribute.writable?
    end
  end
  
  def self.suits_target?(target, target_class)
    self.validBaseClass?(target_class) && !Views.basic_type?(target_class) && !target.viewableAttributes?
  end

  def self.classSelections(host, symbol, targetObj)
    type = if !targetObj
      if host.respond_to?(:model_value?) && symbol
        host.model_value?(symbol).viewable_class
      else
        raise "Can't find a list of classes to use for symbol '#{symbol}' with host '#{host.class}' because the contents of that attribute is nil, and the host does not include SymbolDefaults or does not specify a default for that symbol."
      end
    else
      targetObj.viewable_class
    end
    
    if type.respond_to?(:subclasses)
      begin
        type.first_node_class.subclasses
      rescue EnumerableSubclasses::Exception
        [type]
      end
    else
      [type]
    end
  end

  def self.validBaseClass?(class_type)
    !self.validBaseClass || class_type <= self.validBaseClass
  end
  
  def self.validBaseClass
    @validBaseClass
  end

  def self.validBaseClass=(baseClass)
    @validBaseClass = baseClass
  end
  
protected
  def createDefaultObject(cls)
    if cls.respond_to?(:createDefault)
      View.send_authoritive(true, cls, :createDefault)
    else
      View.send_authoritive(true, cls, :new)
    end
  end
end

module Views
  def self.customClassSelector(baseClass, choices = baseClass.subclasses)
    # tbd: potential memory leak here
    newClass = Class.new(ViewAllAttributesClassSelector)
    newClass.validBaseClass = baseClass
    newClass.classSelections = choices
    newClass
  end
end
