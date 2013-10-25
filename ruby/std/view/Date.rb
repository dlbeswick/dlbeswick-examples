require 'std/view/AttributeReferenceTextField'

module Views
  class Date < AttributeReferenceTextField
    suits ::Date
    
    def formats
      ['%Y-%m-%d', '%Y-%m', '%Y'] 
    end
  
    def value_from_s(text)
      formats().each do |format|
        begin
          return ::Date.strptime(text, format)
        rescue ArgumentError
        end
      end
      
      Date.new(1900, 1, 1)
    end
    
    def value_to_s(value)
      value.strftime('%Y-%m-%d')
    end
    
    def self.order
      -0.9
    end
  end
end
