require 'media/metadata_provider/metadata_provider'
require 'std/module_logging'
require 'std/raise'
gem 'discogs-wrapper'
require 'discogs'

module Auanalysis; end
  
class Auanalysis::Discogs < MetadataProvider
  include ModuleLogging
  include Raise
  
  def initialize(app_name = 'David B\'s record collection app')
    super()
    @wrapper = ::Discogs::Wrapper.new(app_name)
    @min_request_period = 1
    @last_request = Time.at(0)
  end
  
  def with_catalogue_id(cat_id)
    search_releases(cat_id)
  end

  def with_artist(artist)
    api_to_app(request{ @wrapper.get_artist(artist.to_s) }.releases)
  end
  
  def with_title(title)
    search_releases(title.to_s)
  end
  
  def with_artist_and_title(artist, title)
    if !artist.various?
      search_releases("#{artist} #{title}")
    else
      with_title(title.to_s)
    end
  end
  
protected
  def request(&block)
    sleep(@min_request_period - [Time.now - @last_request, @min_request_period].min)
    yield
  end
  
  def search_releases(text)
    log { "Search '#{text}'." }
    api_to_app(request { @wrapper.search(text, :type => 'release') } )
  end
  
  def discogs_format_to_app_format(format_result)
    format_name = format_result.name
    
    descriptions = format_result.descriptions
    
    if descriptions
      permutations = []
          
      descriptions.each do |description|
        permutations += [description, format_name].permutation.to_a.collect do |perm|
          perm.join(" ")
        end
      end
      
      permutations << format_name
      
      permutations.each do |e|
        library().formats.to_a.each do |f|
          if /#{Regexp.escape(e)}/i.match(f.name)
            return f
          end
        end  
      end
    end
    
    library().formats['Unknown/Other']
  end
  
  def request_release(release_id)
    log { "Get release id #{release_id}." }
    request { @wrapper.get_release(release_id) }
  end
  
  def release_for_search_result(search_result)
    release_id = File.basename(search_result.uri)
    log { "Get release for search result '(#{release_id}) #{search_result.summary}'." }
    request_release(release_id)
  end
  
  def api_to_app(api_results)
    api_results = if api_results.kind_of?(::Discogs::Search)
      api_results.results('release').collect do |search_result|
        release_for_search_result(search_result)
      end
    else
      api_results
    end
    
    results = []
      
    api_results.each do |api_release|
      begin
        album = Album.new(library())
        album.clear_tracks
        album.catalogue_id = api_release.labels.first.catno if api_release.labels && !api_release.labels.empty?
        album.media_quantity = api_release.formats.first.qty if api_release.formats && !api_release.formats.empty?
        album.title = api_release.title
        album.release_date = api_release.released

        api_release.tracklist.each do |api_track|
          track = Track.new(album)
          track.artist = api_track.artists.first.name if api_track.artists && !api_track.artists.empty?
          track.artist = library().artists.add_or_retrieve(track.artist) if track.artist
          track.title = api_track.title
        end
          
        if album.track_artists.length > 1
          album.artist = Artist.various(library())
        else
          single_track_artist = album.track_artists.first
          
          album.tracks.each do |track|
            track.artist = nil
          end
          
          album.artist = api_release.artists.first.name if api_release.artists && !api_release.artists.empty?
          
          if album.artist
            album.artist = library().artists.add_or_retrieve(album.artist)
          else
            album.artist = single_track_artist
          end
        end
        
        formats = api_release.formats
        if formats && !formats.empty?
          album.format = discogs_format_to_app_format(formats[0])
        end
        
        results << album
      rescue ::Exception=>e
        err { e.to_s + $/ + e.backtrace.join($/) }
      end
    end
    
    results
  end
end
