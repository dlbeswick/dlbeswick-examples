require './Source/Views/musiccollectionview'
require 'std/profile_help'

class MusicCollectionViewRandomBase < MusicCollectionView
	include PlatformGUISpecific

	def initialize(target, parent)
	  # tbd: fix views so that 'generate' only happens once all initialisation is complete
    @setSize = 10
    @current_min_idx = 0
    @current_max_idx = 0
    @tracks = []
      
    super
    
		updateButton
	end
	
	def fillTree
		doneFirst = false
    
		restart()
	end

	def advance_set(amount)
    clear()
    
    current_idx = if amount > 0
      @current_max_idx + amount 
    else
      @current_min_idx + amount
    end
    
    next_idx = [current_idx + amount, @tracks.length].min
    next_idx = [0, next_idx].max
    
    min_idx = [current_idx, next_idx].min
    max_idx = [current_idx, next_idx].max
    
    @tracks[min_idx...max_idx].each do |track_album_tuple|
      addTrackFlat(track_album_tuple.first, trackText(track_album_tuple.first, track_album_tuple.last))
    end
    
    @current_min_idx = min_idx
    @current_max_idx = max_idx
    
    updateButton()
	end
	
  def prevSet
    advance_set(-@setSize)
  end
  
  def nextSet
    advance_set(@setSize)
  end
  
	def restart
		@tracks = all_tracks().sort_by { rand }
		@currentIdx = 0
		nextSet
	end
	
	def updateButton
		PlatformGUI.specific
	end

	def title
		s = "#{@setSize} random items"

		if @tracks.length != 0
			s << " (iterator at #{((@currentIdx.to_f / @tracks.length.to_f) * 100).to_i}%)"
		end

		s
	end

	def trackText(track, album = nil)
	  album = target().albumForTrack(track) if album == nil
		return "[#{album.title}] #{super(track, album)}"
	end
	
  def description
    "#{@setSize} random items"
  end
  
protected
	def canRefresh?
		@currentIdx + @setSize <= @tracks.length
	end
end

class MusicCollectionViewRandomFox < MusicCollectionViewRandomBase
	include PlatformGUISpecific

	def make_controls(parent)
		button = FXButton.new(@packer, "Restart")
		button.layoutHints = LAYOUT_LEFT
		button.create
		button.reparent(@packer, self.tree)
		button.connect(SEL_COMMAND) { 
			restart 
			0
		}
		
		@button = FXButton.new(@packer, "Next #{@setSize}")
		@button.layoutHints = LAYOUT_LEFT
		@button.create
		@button.reparent(@packer, self.tree)
		@button.connect(SEL_COMMAND) { 
			nextSet 
			0
		}
		
		@window.recalc
		super
	end

	def updateButton
		@button.enabled = canRefresh?
	end
end

class MusicCollectionViewRandomWx < MusicCollectionViewRandomBase
	include PlatformGUISpecific

	def make_controls
		parent = self
		
		sizer = Wx::BoxSizer.new(Wx::HORIZONTAL)
		parent.sizer.add(sizer)
		
		@button = Wx::Button.new(parent, -1, "Restart") do |button|
			button.parent.evt_button(button) do 
				restart 
				true
			end
			
			sizer.add(button)
			
			self
		end
		
		Wx::Button.new(parent, -1, "Next #{@setSize}") do |button|
			button.parent.evt_button(button) do 
				nextSet()
				true
			end

			sizer.add(button)
		end
		
    Wx::Button.new(parent, -1, "Prev #{@setSize}") do |button|
      button.parent.evt_button(button) do 
        prevSet() 
        true
      end

      sizer.add(button)
    end
    
		super
	end

  def on_tree_key_down(event)
    if event.key_code == 'n'.ord
      nextSet
      return true
    elsif
      event.key_code == 'p'.ord
      prevSet
      return true
    end

    super
  end
  
	def updateButton
		@button.enable(canRefresh?)
	end
end
