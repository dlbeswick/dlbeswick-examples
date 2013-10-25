require 'std/view/AttributeReferenceTextField'

class ViewTime < Views::AttributeReferenceTextField
  suits ::Time
  
  def format
    '%Y-%m-%d'
  end

  def value_from_s(text)
    #Time.at(::Date.strftime(text, format()) - Date.new(1970,1,1), 60 * 60 * 24)
  end
  
  def value_to_s(value)
    value.strftime(format())
  end
end
