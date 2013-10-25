require 'media/library'
require 'media/library_importer'
require 'transform/batch_exporter'
require 'std/view/save_button'
require 'std/DataListbox'

# tbd: generalise
class ViewLibraryFind < ViewBasicAttributeReference
  unenumerable

  suits(ViewAllAttributes)
  
  def make_controls
    @input = makeText(self, '') do |text|
      @list.data = target().target().source_albums.find_all { |x| x.to_s =~ /#{text}/i }
    end
    layoutFillHorizontal(@input)
    
    @list = DataListbox.new(self) do |data|
      target().view_for_symbol(:source_albums).view_index(target().target().source_albums.index(data))
    end
    layoutFillVertical(@list.gui)
  end
end

class ViewLibrary < Views::UserActionsPopup
  include Views::SaveButton
  
  suits_class(Library)

  def make_controls
    makeSaveButton
    super
  end

  def batch_export
    ViewContainers::Single.new(BatchExporter.new(target()), parent(), ViewBatchExporter)
  end
  
  def find
    ViewContainers::Single.new(target_view(), parent(), ViewLibraryFind)
  end
  
  def import_csv
    path = GUI.user_get_file(self, "Select CSV file.", Path.new($0).no_file)
    if path
      LibraryImporterCSV.new.load(path.to_file)
    end
  end
    
  def process_outdated_albums
  end
  
  def update_library
    target().update
    target().modified(self)
  end

  if PlatformOS.development?
    def update_library_test
      target().source_albums.clear
      target().add_source_data_from_path(Path.new('d:/data/recordings/Natalie Cole/Good To Be Back'))
      target().update(2)
      target().modified(self)
    end
  end
end
