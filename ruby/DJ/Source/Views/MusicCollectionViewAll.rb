require './Source/Views/musiccollectionview'
require './Source/progress_wx'
require 'std/DataCombo'

class MusicCollectionViewAll < MusicCollectionView
  # resetTree must be called if you wish changes to be reflected in the tree control.
  attr_reader :should_randomize
  
  def make_controls
    @tracks = []

    @order_list = DataCombo.new(
      self, 
      [['Album', :album], 
        ['Random', :random], 
        ['Index', :index]],
      :album
      ) do |text, data|
      @order = data
      resetTree()
    end
    
    layoutAddVertical @order_list.gui

    super
    
    @order = :album
    resetTree()
  end

  def on_tree_key_down(event)
    # wx linux interprets CTRL-R as ASCII char 18
    is_ctrl_r = (event.control_down && event.key_code == 'r'.ord) ||
      event.key_code == 18
      
    if is_ctrl_r
      @want_randomize = !@want_randomize
      on_order_state_changed()
      resetTree()
      return true
    end

    # go home
    if event.key_code == 'h'.ord
      @tree.select_item(@tree.first_child(@root))
      return true
    end
    
    super
  end
  
  def _track_text(track, album = nil)
    "#{track.title} - #{track.artist} [#{album.title}]"
  end
    
  def all_sorted_tracks
    if @tracks_cached_order != @order
      @tracks = nil
    end

    @tracks ||= begin#||= Progress.run("Sorting tracks for music collection view") do
      tracks = case @order 
      when :random
        all_tracks().shuffle
      when :album
        # sort by artist, then album
        all_tracks().sort_by { |track_tuple| [track_artist(track_tuple.first).downcase, track_tuple.last.object_id] }
      when :index
        all_tracks()
      end
      
      @tracks_cached_order = @order

      tracks
    end
  end
  
  def make_filter_tracks
  end
  
  def filter_by(text)
    return if @filter_text && text.downcase == @filter_text.downcase
    
    @filter_tracks = if text.empty?
      nil
    else
      test_text = text.downcase

      all_sorted_tracks().select do |track_tuple|
        trackText(track_tuple.first, track_tuple.last).downcase.include?(test_text)
      end
    end
    
    @filter_text = text
  end
  
  def onRebuild
    @tracks = nil
    super
  end
  
  def tracks_for_tree
    if @filter_tracks
      @filter_tracks
    else
      all_sorted_tracks()
    end
  end
  
  def resetTree
    @last_selected_data = @tree.item_data(@tree.selection)
    super
  end
  
	def fillTree
    iteration_timeslice = 25
    
    tracks = tracks_for_tree()
    
    idx = 0
    
    ProgressStdoutTimerWx.new(tracks.length, "Filling music collection view") do |progress|
      if destroyed?
        log { "Aborting music collection fill, view destroyed." }
        break
      end
      
      element_iteration_amount = 100
      
      until element_iteration_amount == 0 || idx == tracks.length
        track = tracks[idx].first
        track_text = trackText(tracks[idx].first, tracks[idx].last)
        
        addTrackFlat(track, track_text, false)
        
        # try to keep the most recent selection
        if @last_selected_data == track
          @tree.select_item(@tree.last_child(@root))
        end
        
        idx += 1
        element_iteration_amount -= 1
      end
      
      if idx != tracks.length
        progress << element_iteration_amount
        progress.next(iteration_timeslice)
      end
    end
  end

  def description
    'All music'
  end
  
protected
  def order
    if @want_randomize
      :random
    else
      @order_list.selected_data
    end
  end

  def on_order_state_changed
    @order_list.select(order())
  end
end
