require './Source/djapp'
require 'std/EnumerableSubclasses'
require 'std/view/MultipleViewsTabbed'
require 'std/view/View'

class MusicCollectionListViewWx < ViewBasicAttributeReference
  include PlatformGUISpecific
  
  suits MusicCollectionList
	
  def request_find_element_interaction
    @text_ctrl.set_focus
    @text_ctrl.set_selection(-1, -1)
  end
  
  def request_next_view
    notebook = visible_tabbed_view().notebook
    notebook.selection = (notebook.selection + 1) % notebook.page_count  
    visible_collection_view().request_track_selection_element_interaction(false)
  end
  
protected
  def visible_tabbed_view
    @collection_views[@choicebook.selection]
  end
  
  def visible_collection_view
    @collection_views[@choicebook.selection].visible_view
  end
  
  def generateExecute
    super
    
    collection_list = target()
    
    @collection_views = []
    collection_views = @collection_views

    text_ctrl_enter_proc = proc do |text|
      @text_ctrl.set_selection(-1, -1)
      visible_collection_view().filter_by(text)
      visible_collection_view().resetTree()
    end

    text_ctrl_char_proc = proc do |key_code, is_control|
      if key_code == Wx::K_DOWN
        visible_collection_view().request_track_selection_element_interaction(true)
        true
      else
        false
      end
    end
          
    @text_ctrl = make_text_with_char_events(self, "", 15, text_ctrl_enter_proc, text_ctrl_char_proc)
    layoutAddVertical @text_ctrl

    music_collection_view = self
    
    @choicebook = Wx::Choicebook.new(self) do
      collection_list.each do |collection|
        Wx::Panel.new(self) do
          self.sizer = Wx::BoxSizer.new(Wx::VERTICAL)
          
          new_view = Views::MultipleViewsTabbed.new(AttributeReference::ObjectReadOnly.new(collection), self, MusicCollectionView.subclasses)
          
          new_view.views.each do |view| 
            view.extend(MusicCollectionViewForListView)
            view.post_initialize(music_collection_view, )
          end
            
          collection_views << new_view
           
          self.sizer.add(new_view, 1, Wx::GROW)
         
          self.parent.add_page(self, collection.viewable_description)
        end
      end

      self.parent.sizer.add(self, 1, Wx::GROW)

      self
    end
  end
end

class MusicCollectionViewBase < ViewBasicAttributeReference
  include EnumerableSubclasses
  abstract

  suits MusicCollection
  
  attr_reader :building
  attr_reader :index
  @@index = 0

  # Retrieves all music tracks from the server process and caches them for
  # speed.
  # Results are in tupes of [track, track_album].
  # Result is immutable.
  def all_tracks
    @all_tracks ||= [nil].extend(Mutex_m)
	
    @all_tracks.synchronize do
      if @all_tracks == [nil]
        element_iteration_amount = 1000
        
        fill_idx = 0
        @all_tracks = [].extend(Mutex_m)
        
        Progress.loop("Getting all tracks for music collection view") do |progress|
          fill_idx, next_tracks = target().more_tracks_with_albums(fill_idx, element_iteration_amount)
          
          @all_tracks.concat(next_tracks)
          
          progress << next_tracks.length 
          
          if !fill_idx
            break
          end
        end
        
        @all_tracks.freeze
      end
    end
  
    @all_tracks
  end
  
  def filter_by(text)
  end

  # Called when a user wishes for the control to prepare itself for efficient interaction with
  # the track selection element. This typically would involve focusing the element and setting
  # an appropriate selection state.  
  def request_track_selection_element_interaction
  end
    
  def selected_track?
    !selected_track().nil?
  end
  
  def selected_track
    PlatformGUI.specific
  end
  
  # Results are cached to avoid server requests.
  def track_artist(track)
    @track_artist_cache ||= {}
    @track_artist_cache[track.object_id] ||= begin
      track.artist
    end
  end
  
  # Results are cached. Don't override this, override _track_text
  def trackText(track, album = nil)
    @track_text_cache ||= {}
    @track_text_cache[track.object_id] ||= begin
      _track_text(track, album)
    end
  end

  def _track_text(track, album = nil)
    album = target().albumForTrack(track) if album == nil
    
    if album.va?
      "#{track.artist} - #{track.title}"
    else
      track.title
    end
  end
  
  def addAlbumItem(album_title, sorted=true)
    item = @albumToTreeItem[album_title]
  
    if !item
      item = addTreeItem(nil, nil, album_title, sorted)
      @albumToTreeItem[album_title] = item
    else
      raise "Album '#{album_title}' already added."
    end
    
    item
  end
  
	def addTrack(track, track_text = nil, sorted=true)
		album = "Unknown Album" if !track.albumTitle
		
		track_text ||= trackText(track)
		  
		addTreeItem(albumItem(track.albumTitle), track, track_text, sorted)
	end
	
	def addTrackFlat(track, track_text=nil, sorted=true)
    track_text ||= trackText(track)
    
		addTreeItem(nil, track, track_text, sorted)
	end

	def albumItem(albumTitle)
		return nil if !albumTitle
	
		item = nil
		
		item = @albumToTreeItem[albumTitle]
	
		if !item
		  item = addAlbumItem(albumTitle)
		end
		
		item
	end
	
	# Clears the display of tracks only, and not any other internal state.
	def clear
		@albumToTreeItem = {}
		clearTree
	end

	def description
		super
	end
	
	def has_album?(album_title)
    @albumToTreeItem[albumTitle]
	end

  def on_tree_key_down(event)
    false
  end
  
protected
	def initialized
    @building = false

    @index = @@index
    @@index+=1

    @albumToTreeItem = {}
    @albumToTreeItem.extend(Mutex_m)
    
	  @build_proc = proc { |status| GUI.ensure_main_thread { onBuildStatus(status) } }
    target().notify_listen(MusicCollection::NotifyBuildStatus, self, &@build_proc)
    
	  @rebuild_proc = proc { |status| GUI.ensure_main_thread { onRebuild } }
    target().notify_listen(MusicCollection::NotifyRebuild, self, &@rebuild_proc)
  end
  
	def generateExecute
	  super
		clearTree
		resetTree
	end

	def addTreeItem(parent, data, text, sorted)
		PlatformGUI.specific
	end
	
	def clearTree
    PlatformGUI.specific
	end

	def sort
		PlatformGUI.specific
	end

  def fillTree
  	Kernel.raise "Implement in subclasses (#{self.class})."
  	# Example:
  	#	target().tracks.each do |track|
  	#		addTrack(track)
  	#	end
  end

	def updateRefreshButton(enabled)
		PlatformGUI.specific
	end

  def onRefresh
    target().rebuild
    onRebuild
  end

	def onRebuild
		@resetTreeThread.exit if @resetTreeThread
	  @all_tracks.synchronize { @all_tracks = [nil].extend(Mutex_m) }
    @track_text_cache = nil
    @track_artist_cache = nil
		@building = false
		resetTree()
	end
	
	def onBuildStatus(status)
		#resetTree if status
		updateRefreshButton(!status)
	end

	def resetTree
		return if @building
		
		clearTree
		
		@albumToTreeItem.clear
		@building = true
		descriptionChanged

		fillTree
		
		@building = false
		descriptionChanged
	end
end

class MusicCollectionViewWx < MusicCollectionViewBase
  include PlatformGUISpecific
  include Abstract
  
	def make_controls
		control_parent = self

		@refreshButton = Wx::Button.new(control_parent, -1, "Refresh")
		@refreshButton.parent.sizer.add(@refreshButton)
		@refreshButton.parent.evt_button(@refreshButton) do
      onRefresh
    end
		
		@tree = Wx::TreeCtrl.new(control_parent, :style => Wx::TR_HAS_BUTTONS | Wx::TR_HIDE_ROOT)
    @root = @tree.add_root("Root")
		@tree.parent.sizer.add(@tree, 1, Wx::EXPAND)

#		class << @tree
#			def on_compare_items(item1, item2)
#				if self.item_data(item1) == nil || self.item_data(item2) == nil
#					return self.item_text(item1).downcase <=> self.item_text(item2).downcase
#				else
#					return self.item_data(item1) <=> self.item_data(item2)
#				end
#			end
#		end
    
    @tree.parent.evt_tree_key_down(@tree) do |event|
      should_skip = on_tree_key_down(event.key_event) == false 
      if should_skip
        event.skip
      end
    end
    
    @tree.parent.evt_tree_sel_changed(@tree) do
      @tree_last_item_id = @tree.get_selection
    end  
    
		# Without this line, the treectrl's size is set to fill the whole space taken up by all of
		# its items, creating a very large vertical client space in the window.
    sizer().set_size_hints(self)
    
    #@tree.parent.evt_tree_begin_drag(@tree) do |evt|
    #  data = @tree.item_data(evt.item)
    #  if data
    #    $djgui.dragAndDrop.dragStartData(DNDData::Track.new(data))
    #  end
    #end
    
    def @tree.get_effective_min_size
      parent().client_size
    end
    
    super
	end

  def selected_track
    @tree.item_data(@tree.selection)
  end
  
  def request_track_selection_element_interaction(select_first_item)
    @tree.set_focus
    
    item_to_select = if select_first_item || !@tree_last_item_id
      @tree.first_visible_item
    else
      @tree_last_item_id
    end
    
    @tree.select_item(item_to_select)
  end
  
  def resetTree
    guiUpdate do
      super
    end
  end
  
protected
	def addTreeItem(parent, data, text, sorted)
	  @tree.expand(@root)
	  
		if parent == nil
			parent = @root
		end

		if sorted
  		children = @tree.children(parent)
  
  		idx = if data
  			Algorithm.lower_bound(children, data) do |lhs, rhs|
  				lhs <=> @tree.get_item_data(rhs)
  			end
  		else
  			Algorithm.lower_bound(children, text) do |lhs, rhs|
  				lhs <=> @tree.get_item_text(rhs)
  			end
  		end
  		
  		if idx == children.size
  			@tree.append_item(parent, text, -1, -1, data)
  		else
  			@tree.insert_item_before(parent, idx, text, -1, -1, data)
  		end
		else
      @tree.append_item(parent, text, -1, -1, data)
		end
	end
  
	def clearTree
		raise "No tree has been created (#{self.class})" if !@tree
		@tree.delete_all_items
    @root = @tree.add_root("Root")
	end

	def sort
		@tree.sort_children(@root)
	end

	def updateRefreshButton(enabled)
	end
end

# Can be included in a music collection view when that view is intended to be part of a MusicCollectionListView.
# post_initialize must be called after view construction.
module MusicCollectionViewForListViewBase
  include PlatformGUISpecific
  
  def post_initialize(music_collection_list_view)
    @music_collection_list_view = music_collection_list_view
  end
end

module MusicCollectionViewForListViewWx
  include PlatformGUISpecific
  include MusicCollectionViewForListViewBase
  
  def on_tree_key_down(event)
    # wx linux interprets CTRL-F as ASCII char 6
    is_ctrl_f = (event.control_down && event.key_code == 'f'.ord) ||
      event.key_code == 6
    is_ctrl_g = (event.control_down && event.key_code == 'g'.ord) ||
      event.key_code == 7
      
    if is_ctrl_f
      @music_collection_list_view.request_find_element_interaction
      return true
    end
    
    # wx linux interprets CTRL-A as ASCII char 1
    is_ctrl_a = (event.control_down && event.key_code == 'a'.ord) ||
      event.key_code == 1

    if is_ctrl_a
      # mistake protection: the user isn't allowed to replace a playing track so that the
      # non-crossfaded track isn't accidentally changed.
      if !$djgui.app.sourceA.playing? && selected_track?
        $djgui.app.sourceA.open(selected_track())
        return true
      end
    end
    
    # wx linux interprets CTRL-B as ASCII char 2
    is_ctrl_b = (event.control_down && event.key_code == 'b'.ord) ||
      event.key_code == 2
      
    if is_ctrl_b
      # mistake protection: the user isn't allowed to replace a playing track so that the
      # non-crossfaded track isn't accidentally changed.
      if !$djgui.app.sourceB.playing? && selected_track?
        $djgui.app.sourceB.open(selected_track()) if selected_track?
        return true
      end
    end
    
    # wx linux interprets CTRL-L as ASCII char 12
    is_ctrl_l = (event.control_down && event.key_code == 'l'.ord) ||
      event.key_code == 12
      
    if is_ctrl_l || is_ctrl_g
      if selected_track? && $djgui.app.focused_source != nil
        target_sources = $djgui.app.sources
  
        non_focused_source = target_sources.find { |source| !source.drb_equal?($djgui.app.focused_source) }

        if non_focused_source
          non_focused_source.open(selected_track()) 
        end
      end

      return true
    end
    
    is_tab = event.key_code == 9
    if is_tab
      @music_collection_list_view.request_next_view
      return true
    end
    
    super
  end
end
