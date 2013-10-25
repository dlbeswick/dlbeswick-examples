class FileView
	attr_reader :fileWindow
	attr_reader :fileList
	
	def initialize(mainWindow, path)
		fileWindow = FXDialogBox.new(mainWindow, "Files", DECOR_ALL & ~DECOR_CLOSE, 1280 - 400, 0, 400, 800)
		fileList = FXFileList.new(fileWindow)
		fileList.directory = path
		fileList.connect(SEL_COMMAND) { |sender, sel, ptr|
			if fileList.itemDirectory?(ptr)
				fileList.directory = fileList.itemPathname(ptr)
			end
		}
		fileWindow.show
		fileWindow.create
	end
end