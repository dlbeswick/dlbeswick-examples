require 'media/metadata_provider/metadata_provider'
require 'net/http'
require 'cgi'
require 'module_logging'
require 'raise'
require 'rexml/document'
require 'zlib'

class Discogs < MetadataProvider
  include ModuleLogging
  include Raise
  
  def initialize(api_key = 'fb10f6fd5e')
    super()
    @api_key = api_key
  end
  
  def with_catalogue_id(cat_id)
    search(cat_id)
  end

  def with_artist(artist)
    search("#{artist}")
  end
  
  def with_title(title)
    search("#{title}")
  end
  
  def with_artist_and_title(artist, title)
    if !artist.various?
      search("#{artist} #{title}")
    else
      search("#{title}")
    end
  end
  
protected
  def discogs_format_to_app_format(format_xml)
    format_name = format_xml.attributes['name']
    
    descriptions = format_xml.elements.to_a('./descriptions/description').collect { |e| e.text }
    
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
    
    library().formats['Unknown/Other']
  end
  
  def search(query, type='all')
    return [] if query.empty?
      
    http = Net::HTTP.new('www.discogs.com')
    
    query = CGI.escape(query)
    
    path_no_key = "/search?type=#{type}&q=#{query}"
    
    results = []
    
    request(http, path_no_key).get_elements('searchresults/result/uri').each do |element|
      # both 'master' (artist) and 'release' info is returned from the search. only process release.
      if element.parent.attributes['type'] == 'release'
        match = /(http\:\/\/.+?)(\/.+?)(\/.+)/.match(element.text)
        if !match
          raise "Couldn't extract URI from search result. (#{element.text})"
        end
        
        begin
          xml = request(http, match[3]).get_elements("./release").first
          
          album = Album.new(library())
          album.clear_tracks
          album.catalogue_id = get_attribute(xml, "./labels/label", "catno")
          album.media_quantity = get_attribute(xml, "./formats/format", "qty")
          album.title = get_text(xml, "./title")
          album.release_date = get_text(xml, "./released")
          
          xml.each_element("./tracklist/track") do |element|
            track = Track.new(album)
            track.artist = get_artist_text(element, "./artists/artist/name")
            track.artist = library().artists.add_or_retrieve(track.artist) if track.artist
            track.title = get_text(element, "./title")
          end
          
          if album.track_artists.length > 1
            album.artist = Artist.various(library())
          else
            single_track_artist = album.track_artists.first
            
            album.tracks.each do |track|
              track.artist = nil
            end
            
            album.artist = get_artist_text(xml, "./artists/artist/name")
            
            if album.artist
              album.artist = library().artists.add_or_retrieve(album.artist)
            else
              album.artist = single_track_artist
            end
          end
          
          formats = xml.get_elements("./formats/format")
          if !formats.empty?
            album.format = discogs_format_to_app_format(formats[0])
          end
          
          results << album
        rescue Exception=>e
          err { e.to_s + $/ + e.backtrace.join($/) }
        end
      end
    end
    
    results
  end
  
  def get_attribute(xml, path, attribute)
    result = xml.get_elements(path).first
    
    if result
      result.attributes[attribute]
    else
      nil
    end
  end
  
  def get_text(xml, path)
    result = xml.get_text(path)
    
    if result
      result.value
    else
      nil
    end
  end
  
  def get_text_no_variant_marker(xml, path)
    result = get_text(xml, path)
    
    if result
      with_variant = /(.+) (\(\d+\))/.match(result)
      if with_variant
        result = with_variant[1]
      end
    end
    
    result
  end

  def get_artist_text(xml, path)
    result = get_text_no_variant_marker(xml, path)

    if result
      with_postfix_the = /(.+), (the)$/i.match(result)
      if with_postfix_the
        result = "#{with_postfix_the[2]} #{with_postfix_the[1]}"
      end
    end
    
    result
  end
  
  def request(http, path_no_key)
    if path_no_key =~ /\?/
      path_no_key += "&f=xml"
    else
      path_no_key += "?f=xml"
    end
    
    path = "#{path_no_key}&api_key=#{@api_key}"
    
    results = []
    
    headers = {
      'Accept-Encoding' => 'gzip',
      'User-Agent' => 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:6.0) Gecko/20100101 Firefox/6.0',
    }
      
    
    log { path }
    
    result = nil
    body = nil
    
    attempts = 0  
    
    begin
      sleep(1) if attempts > 0
      
      result, body = http.get(path, headers)
      attempts += 1
    end while result.kind_of?(Net::HTTPInternalServerError) && attempts < 5

    if !result.kind_of?(Net::HTTPOK)
      raise "HTTP request failed. (#{path_no_key}, #{result})"
    else
      body = begin
        if body[0..4] == "<resp"
          # Plain XML format response
          body
        else
          # Gzip format response
          Zlib::GzipReader.new(StringIO.new(body)).read
        end
      rescue Zlib::GzipFile::Error => e
        raise "Error retrieving XML. (#{path_no_key}, #{e})"
      end
      
      doc = begin
        REXML::Document.new(body)
      rescue REXML::ParseException => e
        raise "Error parsing XML result. (#{path_no_key}, #{e})"
      end
      
      doc.root
    end
  end
end