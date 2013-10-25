require 'media/source_file'

class SourceFileAudacity < SourceFile
  def self.extension
    'aup'
  end

  # tbd: move this to a "sourcefilesoundfile" superclass.  
  def extract_raw_sound_data(source_track)
    raise "Invalid sample range '#{source_track.sample_range}'." if source_track.sample_range.length == 0
    
    raw_data_files = []
    source_track.source_file.sound_file.channels.times do |i|
      raw_data_files << DLB::Tempfile.open("tmp_channel_#{i}")
    end
    
    sound_file = source_track.source_file.sound_file

    reader = sound_file.sequential_reader
    reader.seek(source_track.sample_range.first)
    
    sample_count = [source_track.sample_range.length, sound_file.length_samples - source_track.sample_range.first].min
    
    Progress.run(sample_count, "Writing samples #{source_track.sample_range} to #{raw_data_files.collect{ |f| f.path }.join(',')}") do |progress|
      t = Time.now
      
      while sample_count > 0 do
        read_samples = [2**20 / reader.sound_file.bytes_per_sample, source_track.sample_range.normalise_exclusive.last - reader.tell].min
        data = reader.read(read_samples)
        break if !data
        
        data.zip(raw_data_files) do |channel, io|
          io.write(channel)
        end

        sample_count -= data.first.length
        progress << data.first.length
      end
    end
    
    raw_data_files.each { |io| io.keep!; io.close }
    raw_data_files
  end
  
  def extract_source_tracks
    idx = 0

    begin
      aup_file = sound_file()
    rescue SoundFile::Exception=>e
      raise ExtractTracksException, e.to_s
    end
    
    labels = aup_file.labels

    raise ExtractTracksException, "No source album defined." if !source_album()
    raise ExtractTracksException, "Not enough labels defined." if labels.length < 2 
    
    result = []
      
    labels[0..-2].each_with_index do |l, i|
      if valid_track?(l.title)
        source_track = SourceTrack.new(source_album(), self, l.title)
        
        new_boundary = aup_file.seconds_to_samples(l.t).round
        next_boundary = aup_file.seconds_to_samples(labels[i+1].t).round

        # discount zero-length boundaries (discards tracks created by multiple boundary markers
        # on the same sample; those tracks are nearly always erroneous)
        next if new_boundary == next_boundary
        
        source_track.sample_range = (new_boundary...next_boundary)
        
        result << source_track
      end
    end
    
    result
  end

  # tbd: remove this, not needed for multithreaded export
  def post_export(raw_data_file_paths)
    raw_data_file_paths.each { |path| path.delete }
  end
  
  def lossy?
    false
  end
  
  def sound_file
    SoundFileAudacity.new(path().to_file('rb'), path())
  end
  
  def valid_track?(label_title)
    /\w/.match(label_title) && !/end/i.match(label_title)
  end
end
