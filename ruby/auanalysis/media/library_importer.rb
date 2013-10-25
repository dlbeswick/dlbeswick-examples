class LibraryImporter
  include Viewable
end

class LibraryImporterCSV < LibraryImporter
  attr_accessor :field_mapping
  
  def load(io, delimiter=',', text_delimiter='"')
    header_text = io.readline.chomp

    delimiter = Regexp.escape(delimiter)
    text_delimiter = Regexp.escape(text_delimiter)
    
    fields = header_text.scan(/#{text_delimiter}(.*?)#{text_delimiter}#{delimiter}?/).collect do |result|
      result.first
    end
    
    @field_mapping = {}
      
    fields.each do |field|
      @field_mapping[field] = nil
    end
  end
  
  def import_to(library)
  end
end