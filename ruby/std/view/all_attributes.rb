require 'std/view/View'

class ViewAllAttributesCommon < View
  include ModuleLogging
  include PlatformGUISpecific
  
  attr_writer :border
  
  self.log=false
  
  def initialize(target, parent, border=true)
    @border = border

    super(target, parent)
  end
  
  def initialized
    @views = []
    @labels = []
  end
  
  def border
    @border && !border_text.empty?
  end
  
  def onViewableDependencyEvent(record)
    super
    
    GUI.ensure_main_thread do
      if @views.find { |x| x.equal?(record.who) }
        guiAdded()
      end
    end
  end

  # tbd: getting ugly, this and 'attributeTarget' is to allow use with AttributeReferences or plain
  # object references. Find a better way.
  # note: this may have been solved by the redefinition of the 'target' method in AttributeReferenceView module.
  def parentAttributeForSymbols
    nil
  end
  
  def hasWriter?(symbol)
    target().respond_to?("#{symbol.to_s[1..-1]}=")
  end
  
  def view_for_symbol(symbol)
    @views.find { |view| view.attribute().symbol == symbol }
  end
  
  def self.order
    -1
  end

  def self.suits_target?(target, target_class)
    !(target_class <= ::AttributeReference::Base) && !Views.basic_type?(target_class) && !target.viewableAttributes? 
  end

protected
  def border_text
    target().viewable_summary
  end
  
  def circular?(obj)
    Algorithm.recurse(self, :parent) do |parent|
      # fix this, remove DRb considerations
      if (!parent.respond_to?(:viewable_kind_of?) && parent.kind_of?(DRb::DRbObject))
        raise "DRbObject encountered not supporting viewable_kind_of?, can't determine type. (#{parent})"
      end
      
      if ((parent.respond_to?(:viewable_kind_of?) && parent.viewable_kind_of?(View)) \
        || (parent.kind_of?(View))) \
        && parent.target() == obj
        return true 
      end
    end
    
    false
  end
  
  def generateExecute
    guiUpdate() do
      @viewableAttributes = nil
      
      while !@views.empty?
        @views.shift.view_destroy
        #GC.start
      end
      
      while !@labels.empty?
        GUI.destroy_window(@labels.shift) 
        #GC.start
      end
      
      ViewAllAttributesCommon.log { "Viewing attributes for #{target().viewable_debuginfo}." }

      attributes = viewableAttributes()

      attributes.each do |v|
        ViewAllAttributesCommon.log { "Attribute: #{v}." }
        addViewForSymbol(v[1..-1].intern)
      end
    end
  end
  
  def hasReader(symbol)
    target().respond_to?(symbol[1..-1])
  end

  def self.store_viewable_attributes(classType, attributes)
    @stored_viewable_attributes ||= {}
    @stored_viewable_attributes[classType] = attributes
  end

  def self.stored_viewable_attributes(classType)
    @stored_viewable_attributes ||= {}
    @stored_viewable_attributes[classType]
  end
  
  def viewForSymbol(symbol)
    View.viewAttribute(symbol, target(), self, parentAttributeForSymbols())
  end
  
  def addViewForSymbol(symbol)
    layoutNextRow
    
    text = makeStaticText(self, symbol.to_s)
    layoutAddHorizontal(text)
    @labels << text
    
    newView = viewForSymbol(symbol) 
    layoutAddHorizontal(newView, 5)
    @views << newView
  end
  
  def viewableAttributes
    # tbd: this is an optimisation. can stored attributes change?
    attributes = ViewAllAttributesCommon.stored_viewable_attributes(target().viewable_class)
    
    if !attributes
      attributes = target().viewableAttributes
      ViewAllAttributesCommon.log { "#{target().viewable_debuginfo} has attributes #{attributes.inspect}." }
      
      attributes.delete_if do |v|
        attributeObj = if v[0..0] == "@"
          target().instance_variable_get(v)
        else
          target().send(v)
        end
        
        # collect conditions in a list to aid logging
        # conditions are or-ed together
        # An attributes is only displayed if:
        # It has a reader
        # It has a writer if it's a basic type
        # If it is not a circular reference to the attribute object
        # If it is not a Proc object
        # If it is not a basic type, it must respond to 'viewableAttributes'
        deletionConditions = [
          'attributeObj.class == Proc',
          '!hasReader(v)',
          'circular?(attributeObj)',
          '!Views.basic_type?(attributeObj) && !attributeObj.respond_to?(:viewableAttributes)',
        ]
        
        delete = false
        
        deletionConditions.each do |c|
          if eval(c)
            ViewAllAttributesCommon.log { "Deleting attribute #{v}: #{c}." }
            delete = true
            break
          end
        end
        
        delete
      end
      
      attributes.sort!
      
      ViewAllAttributesCommon.store_viewable_attributes(target().viewable_class, attributes)
    end
 
    attributes
  end
end

class ViewAllAttributesWx < ViewAllAttributesCommon
  include PlatformGUISpecific

  def makeSizer(target)
  end
  
  def make_controls
    return if viewableAttributes().empty?

    if border()
      self.sizer ||= Wx::StaticBoxSizer.new(Wx::VERTICAL, self, border_text())
    else
      self.sizer ||= Wx::BoxSizer.new(Wx::VERTICAL)
    end
    
    super
  end
end
  
class ViewAllAttributesReference < ViewAllAttributes
  include Views::AttributeReferenceView

  def self.suits_attribute?(attribute)
    true
  end
  
  def self.suits_target?(target, target_class)
    !Views.basic_type?(target_class) && !target.viewableAttributes? 
  end
  
  def self.order
    -1
  end
end
