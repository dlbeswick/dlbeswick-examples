require 'std/platform'
require 'std/view/View'
require 'std/view/labeled_field'

module Views
  class AttributeReferenceTextFieldCommon < ViewBasicAttributeReference
    abstract
    include PlatformGUISpecific
    
    attr_writer :onRightClick
  
    def initialize(target, parentWindow)
      super
    end
    
    def commit(text)
      self.targetValue = value_from_s(text)
    end
    
    def description
      @description
    end
    
    def showLabel=(b)
      PlatformGUI.specific
    end

    def targetValue=(value)
      @target.set(value)
      # this view is commonly a child of AllAttributesView.
      # clean the parent so that this control is not destroyed when AllAttributesView regenerates itself.
      # tbd: this should probably be fixed in AllAttributesView. AllAttributesView should reset the
      # contents of its children instead of destroying them.
      parent.clean
      @target.modified(self)
    end
    
    def targetValue
      @target.get
    end
    
    def updateValue
      PlatformGUI.specific
    end
    
    def value_to_s(value)
      value.to_s
    end
    
    def value_from_s(text)
      raise "Implement this."
    end
    
    def self.order
      -2
    end
  
  protected
    def onRightClick
      @onRightClick.call if @onRightClick
    end
    
    def screenWidth(text)
      PlatformGUI.specific
    end
  
    def defaultFieldWidth(value)
      if value.viewable_kind_of?(String) && value.empty?
        screenWidth('W' * 10)
      else
        screenWidth(value_to_s(value) + 'WW')
      end
    end
  
    def generateExecute
      super
      updateValue()
      fitToValue()
      guiAdded()
    end
  end

  class AttributeReferenceTextFieldWx < AttributeReferenceTextFieldCommon
    abstract
    include PlatformGUISpecific
    
    def make_controls
      @text = if target_editable?
        Wx::TextCtrl.new(self, -1, '', Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE, Wx::TE_PROCESS_ENTER) do |t|
          t.evt_char do |event|
            if event.key_code == Wx::K_RETURN || event.key_code == Wx::K_NUMPAD_ENTER
              commit(t.value)
              if !destroyed?
                fitToValue
                t.insertion_point = 0
                t.set_selection(-1, -1)
              end
            else
              event.skip
            end
          end
        end
      else
        Wx::StaticText.new(self, -1, '')
      end

      layoutAddHorizontal(@text)

      @text.evt_right_down do
        onRightClick
      end
    end
    
    def fitToValue
      if @text && !destroyed?
        extent = defaultFieldWidth(targetValue())
        @text.min_size = Wx::Size.new(extent[0], extent[1] + 5)
      end
    end
    
    def screenWidth(text)
      text_extent(text)
    end

    def textValue
      if @text.respond_to?(:get_value)
        @text.value
      else
        @text.label
      end
    end
  
    def updateValue
      value = value_to_s(self.targetValue)
      if @text.respond_to?(:set_value)
        @text.value = value
      else
        @text.label = value
      end
    end
    
  protected
    def target_editable?
      attribute().writable? && Views.type_mutable?(target())
    end    
  end

  class AttributeReferenceTextFieldBasicType < AttributeReferenceTextField
    def value_from_s(text)
      if targetValue.viewable_kind_of?(Integer)
        if text =~ /\./
          return text.to_f
        else
          return text.to_i
        end
      elsif targetValue.viewable_kind_of?(Numeric)
        return text.to_f
      elsif targetValue.viewable_kind_of?(TrueClass) || targetValue.viewable_kind_of?(FalseClass)
        if text =~ /true/i || text == '1'
          return true
        else
          return false
        end
      else
        raise "Don't know how to convert string to type #{@target.class}" if !targetValue.viewable_kind_of?(String)
        return text
      end
    end

    def self.suits_target?(target, target_class)
      Views.basic_type?(target_class) && !target.nil?
    end
  end
end
