require 'std/view/View'

module Views
  class ArrayCommon < View
    include AttributeReferenceView 
    include PlatformGUISpecific
  
    suits ::Array
    
    def make_controls
      @elementViews = []
  
      makeDescription
  
      if self.canAppendEntries?
        makeAddButton
      else
        makeNoTypeText
      end
  
      guiAdded()
    end
  
    def makeDescription
      PlatformGUI.specific
    end
    
    def makeAddButton
      PlatformGUI.specific
    end
    
    def makeNoTypeText
      PlatformGUI.specific
    end
  
    def generateExecute
      @elementViews.delete_if do |v|
        #GC.start
        v.view_destroy
        true
      end
    
      arrayObject = arrayObject()
  
      if !arrayObject.empty? && arrayObject[0]
        @lastRememberedElement = arrayObject[0]
      end

      arrayObject.each_index do |i|
        addElementView(i)
      end
    end
  
    def arrayObject
      if canAppendEntries?
        attribute().get_authoritive
      else
        attribute().get
      end
    end
  
    def canAppendEntries?
      # entries can be appended if there is a default object available, or if it can be inferred from the contents of the array.
      # tbd: default obj can be considered a model obj, so just call default obj
      if attribute().host.respond_to?(:defaultObjForSymbol?) && attribute().host.defaultObjForSymbol?(attribute().symbol)
        true
      elsif attribute().host.respond_to?(:model_value?) && attribute().host.model_value?(attribute().symbol)
        true
      elsif !arrayObject().empty? && SymbolDefaults.defaultTypeForInstance(self.arrayObject[0])
        true
      else
        false
      end
    end
  
    def addNewEntry(beforeIndex=arrayObject().length)
      arrayObject = self.arrayObject
      
      newObj = newObjectFromEntryAt(beforeIndex)
      
      begin
        # SymbolDefaults will usually supply an array with one element for array types, so newObj must be an array 
        raise "Expected an array (#{newObj.class})" if !newObj.viewable_kind_of?(::Array)
        
        newObjectIndexes = (beforeIndex..(beforeIndex + (newObj.length - 1))).to_a
  
        newObj.reverse_each do |e|
          arrayObject().insert(beforeIndex, e)
        end
    
        newObj.each do |i|
          entryAdded(i)
        end
    
        cleanAncestors
  
        arrayObject().modified(ArrayModificationRecord.new(self, newObjectIndexes, 'add'))
        
        newObjectIndexes.each do |i|
          addElementView(i)
          raise "@elementViews.length != arrayObject().length (#{@elementViews.length} != #{arrayObject().length})" if @elementViews.length != arrayObject().length
        end
        
        modified(self)
        guiAdded()
      ensure
        View.release_authoritive(newObj)
      end
    end
  
    def removeEntry(index)
      initialArrayObject = arrayObject.dup
  
      arrayObject = self.arrayObject
      obj = arrayObject[index]
  
      entryRemoving(obj)
  
      raise "The array was modified during execution of 'entryRemoving'." if initialArrayObject != arrayObject
  
      arrayObject.delete_at(index)
      initialArrayObject = arrayObject.dup
  
      entryRemoved(obj)
  
      raise "The array was modified during execution of 'entryRemoved'." if initialArrayObject != arrayObject
  
      cleanAncestors
      target().modified(ArrayModificationRecord.new(self, [index], 'remove'))
      removeElementView(index)
      guiAdded()
    end
  
    def onViewableDependencyEvent(record)
      GUI.ensure_main_thread do
        if record.kind_of?(ArrayModificationRecord) 
          if record.action == 'add'
            record.indexes.each do |i|
              addElementView(i)
              raise "@elementViews.length != arrayObject().length (#{@elementViews.length} != #{arrayObject().length})" if @elementViews.length != arrayObject().length
            end
          elsif record.action == 'remove'
            removeElementView(record.indexes.first)
          elsif record.action == 'replace'
            replaceElementView(record.indexes.first, record.who[record.indexes.first])
          else
            raise "Bad action '#{@record.action}'."
          end
          
          guiAdded()
          modified(record.instigator)
        elsif @elementViews.include?(record.who)
          guiAdded()
        elsif record.who.equal?(target())
          ViewBase.log { "Array object had modify request." }
          if dirty?
            ViewBase.log { "  Dirty." }
            # tbd: this sort of generate will be triggered many times.
            # the entire object will typically be generated again because ultimately the AllAttributes
            # view will be dirty and will rebuild itself. However it should still be valid for an array view
            # to rebuild itself. Should probably batch generation requests and execute them last to first
            # called. As the lastmost object generates, the earlier objects will have been destroyed and so
            # the rebuild will be skipped.
            #generate 
          end
        else
          super
        end
      end
    end
  
    def self.order
      -2
    end
    
  protected
    def newObjectFromEntryAt(index)
      initialArrayObject = arrayObject.dup
      
      newObj = if attribute().host.respond_to?(:model_value?) && attribute().host.model_value?(attribute().symbol) 
        View.send_authoritive(true, attribute().host, :model_value, attribute().symbol)
      elsif attribute().host.respond_to?(:defaultObjForSymbol)
        View.send_authoritive(true, attribute().host, :defaultObjForSymbol, attribute().symbol)
      else
        if !self.arrayObject.empty?
          [View.send_authoritive(true, SymbolDefaults, :defaultTypeForInstance, self.arrayObject.first)]
        end
      end
  
      raise "The array was modified while retrieving the default object." if initialArrayObject != arrayObject()
      
      raise "An array's host must respond to defaultObjForSymbol, or be non-empty. (#{attribute()})" if !newObj
      
      newObj
    end
  
    def addElementView(index)
      raise "index > @elementViews.length (#{index} > #{@elementViews.length})" if index > @elementViews.length
        
      # tbd: use newViewFor, restricted to ArrayElement(Common)
      arrayElementView = ArrayElement.new(self, target(), self)
      
      arrayElementView.view do
        attributeReference = #if attribute().kind_of?(::AttributeReference::AttributeSymbol) \
        #  && !attribute().kind_of?(::AttributeReference::ArrayElementSymbol) # fix: make arrayelementsymbol a type of arrayelement
        #  ::AttributeReference::ArrayElementSymbol.new(attribute().symbol, index, attribute().host, attribute())
        #else
          ::AttributeReference::ArrayElement.new(index, target(), attribute())
        #end
        View.new_for(attributeReference, arrayElementView, possibleElementViewSubclasses(attributeReference))
      end
      
      insertBefore = sizer().children.length
      
      sizer().children.each_with_index do |obj, i|
        if obj.window && obj.window == @elementViews[index]
          insertBefore = i
          break
        end
      end
      
      raise "nil arrayElementView" if !arrayElementView
      
      @elementViews.insert(index, arrayElementView)
      
      layoutAddVertical(arrayElementView, 0, insertBefore)
  
      updateAllIndexes
    end
    
    def replaceElementView(index, object)
      arrayElementView = @elementViews[index]
      
      arrayElementView.view do
        new_view = View.newViewFor(arrayElementView.elementView.attribute(), arrayElementView) 
        guiAdded()
        new_view
      end
    end
  
    def updateAllIndexes
      @elementViews.each_with_index do |obj, i|
        obj.elementView.attribute.idx = i
        obj.updateIndex
      end
    end
    
    def removeElementView(index)
      @elementViews.delete_at(index).view_destroy
      updateAllIndexes
    end
  
    def onElementViewAdded(view)
      PlatformGUI.specific
    end
  
    def possibleElementViewSubclasses(elementObject)
      View.subclasses
    end
  
    def entryAdded(obj)
    end
    
    def entryRemoving(obj)
    end
  
    def entryRemoved(obj)
    end
  end   

  class ArrayElementCommon < View
    include PlatformGUISpecific
    include AttributeReference
    abstract
    
    attr_reader :elementView
  
    def initialize(arrayView, target, parentWindow)
      @arrayView = arrayView
      super(target, parentWindow)
    end
    
    def updateIndex
      PlatformGUI.specific
    end
  
    def view(&view)
      @elementView = createElementView(view)
      updateIndex
      @elementView
    end
  
    def onViewableDependencyEvent(record)
      if record.who == @elementView
        GUI.ensure_main_thread do
          guiAdded()
        end
        
        modified(record)
      end
    end
    
    def self.suits_attribute?(attribute)
      attribute.viewable_kind_of?(AttributeReference::ArrayElement)
    end
    
  protected
    def createElementView(viewProc)
      PlatformGUI.specific
    end
  end

  class ArrayWx < ArrayCommon
    include PlatformGUISpecific
    
    def addElementView(index)
      super
      
      # Scrollbars on ScrolledWindows are not set properly unless this line executes.
      sizer().fit_inside(parent)
    end
    
    def makeDescription
      Wx::StaticText.new(self, -1, attribute().viewable_name) do |ctrl|
        @horz.add(ctrl)
      end
    end
    
    def makeAddButton
      Wx::Button.new(self, -1, '+') do |ctrl|
        @horz.add(ctrl)
        
        ctrl.parent.evt_button(ctrl) do
          addNewEntry
        end
      end
    end
    
    def makeNoTypeText
      Wx::StaticText.new(self, -1, '(array of indeterminate type)') do |ctrl|
        @horz.add(ctrl)
      end
    end
    
    def make_controls
      @horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
      self.sizer.add(@horz)
  
      super
    end
  end
  
  class ArrayElementWx < ArrayElementCommon
    include PlatformGUISpecific
    abstract
    
    def updateIndex
      raise "The view has not yet been created." if !@elementView
      
      text = "[#{@elementView.attribute.idx}]"
      if uses_staticbox?
        @viewSizer.static_box.label = text 
      else
        @idx_textbox.label = text
      end
    end
  
  protected
    def uses_staticbox?
      !PlatformGUI.gtk?
    end
    
    def createElementView(viewProc)
      if !@elementView
        @horz = Wx::BoxSizer.new(Wx::HORIZONTAL)
        self.sizer.add(@horz)
        
        if uses_staticbox?
          @viewSizer = Wx::StaticBoxSizer.new(Wx::VERTICAL, self, '')
        else
          @viewSizer = @horz 
          @idx_textbox = makeStaticText(self, '')
          @horz.add(@idx_textbox)
        end
        
        buttonAdd = Wx::Button.new(self, -1, '+', :style => Wx::BU_EXACTFIT) do |ctrl|
          ctrl.parent.evt_button(ctrl) do 
            @arrayView.addNewEntry(@elementView.attribute.idx)
          end
          
          @horz.add(ctrl)
        end
  
        buttonRemove = Wx::Button.new(self, -1, 'X', :style => Wx::BU_EXACTFIT) do |ctrl|
          ctrl.parent.evt_button(ctrl) do 
            @arrayView.removeEntry(@elementView.attribute.idx)
          end
          
          @horz.add(ctrl)
        end
        
        view = viewProc.call
  
        @horz.add(view)
        @horz.add(@viewSizer) if uses_staticbox?
        
        view
      else
        view = viewProc.call

        @elementView.view_destroy
        @elementView = nil
        
        @viewSizer.add(view)
        @viewSizer.layout
        
        view
      end
    end
  end
  
  class ArrayObjectReferences < Array
    def possibleElementViewSubclasses(elementObject)
      if [::Hash, ::String, ::Array, ::Numeric].any? { |c| elementObject.viewable_kind_of?(c) }
        super
      else
        View.subclasses(AttributeObjectReference)
      end
    end
    
    def self.order
      -10
    end
  end
end

