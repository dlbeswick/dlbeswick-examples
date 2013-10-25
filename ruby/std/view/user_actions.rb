require 'std/abstract'
require 'std/view/View'

module Views
  class UserActions < ViewBasicAttributeReference
    include Abstract
    include EnumerableSubclasses
    include MethodChain

    attr_reader :target_view
        
    class UserActionEntry
      attr_accessor :description
      attr_accessor :symbol_method
      
      def initialize(description, symbol_method)
        @description = description
        @symbol_method = symbol_method
      end
      
      def <=>(other)
        symbol_method() <=> other.symbol_method()
      end
      
      def eql?(other)
        other.viewable_kind_of?(self.class) && symbol_method().eql?(other.symbol_method)
      end
      
      def hash
        symbol_method().hash
      end
      
      def call(target, *args)
        target.send(@symbol_method, *args)
      end
    end
    
    module_chain(self.class, :inherited) do |super_info, child|
      MethodChain.super(self, super_info, child)
      child._super_user_actions = user_actions() if !abstract?
    end
    
    def self._super_user_actions=(useraction_array)
      @super_user_actions = useraction_array
    end
    
    def self.suits_class(class_type)
      suits(class_type)
    end
    
    class << self; attr_reader :custom_view_proc; end
      
    def self.custom_view(&block)
      module_eval do
        @custom_view_proc = block
      end
    end
    
    def self.description_for(symbol_method)
      text = symbol_method.to_s
      (text[0..0].upcase + text[1..-1]).gsub('_', ' ')
    end

    def self.suits_attribute?(attribute)
      true
    end

    def self.user_actions
      if @user_actions.nil?
        @user_actions = valid_action_methods().collect do |method|
          UserActionEntry.new(description_for(method), method)          
        end
        
        if @super_user_actions != nil
          @user_actions.concat(@super_user_actions)
          @user_actions.uniq!
        end
        
        @user_actions.sort!
      end
      
      @user_actions
    end
    
    def self.valid_action_methods
      instance_methods(false) 
    end
    
    # tbd: remove
    def make_controls
      make_controls()
    end
    
    def make_controls
      super
      make_target_view()
    end
        
    def make_target_view
      @target_view = if self.class.custom_view_proc
        instance_eval(&self.class.custom_view_proc)
      else
        new_view = View.new_for(attribute(), self, View.subclasses - UserActions.subclasses)
        layoutAddVertical(new_view)
        new_view
      end
    end
    
    def user_actions
      self.class.user_actions
    end
  end

  class UserActionsButtons < UserActions
    include Abstract

    def make_controls
      user_actions().each do |action|
        button = makeButton(self, action.description) do
          action.call(self)
        end

        layoutAddHorizontal(button)
      end

      layoutNextRow
      
      super      
    end
  end
  
  class UserActionsPopupCommon < UserActions
    include PlatformGUISpecific
    
    def make_controls
      @button = makeButton(self, 'Actions') do
        show_popup
      end

      layoutAddHorizontal(@button)
      layoutNextRow

      super      
    end
    
    def show_popup
      PlatformGUI.specific
    end
  end
  
  class UserActionsPopupWx < UserActionsPopupCommon
    include Abstract
    include PlatformGUISpecific
    
    def show_popup
      if !@menu
        @menu = Wx::Menu.new
        
        user_actions().each_with_index do |action, i|
          @menu.append(i, action.description)
        end
        
        evt_menu(Wx::ID_ANY) do |event|
          user_actions()[event.id].call(self)
        end
      end
      
      popup_menu(@menu)
    end
  end
end
