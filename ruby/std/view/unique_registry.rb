require 'std/unique'

class ViewUniqueRegistry < ViewBasicAttributeReference
  suits UniqueHost::UniqueRegistry
  
  def make_controls
    layoutAddHorizontal(
      @text = makeText(self, '', 10)
    )
      
    layoutAddHorizontal(
      makeButton(self, 'Add') do
        begin
          target().add(@text.value)
          target().modified(self)
        rescue UniqueHost::UniqueRegistry::Exception
          err { "Failed to add '#{@text.value}'" }
        end
      end
    )
  end
  
  def self.order
    -0.9
  end
end
