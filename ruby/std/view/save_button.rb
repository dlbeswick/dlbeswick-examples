require 'std/view/View'

module Views
  module SaveButton
    def makeSaveButton
      button = makeButton(self, 'Save') do
        on_save_button_clicked
      end
      
      layoutAddHorizontal(button)
    end
    
    def on_save_button_clicked 
      target().save
    end
  end
end
