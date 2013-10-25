require 'std/abstract'
require 'std/SafeThread'

# Call default_progress_class= to set the default progress class used for an application.
class Progress
  include Abstract
  
  attr_reader :description
  
  def initialize(total = nil, description = nil, verbose = false, &block)
    @total = total
    @progress = 0.0
    @description = description
    @verbose = verbose
  end

  def self.default_progress_class=(progress_class)
    @default_progress_class = progress_class
  end
  
  def self.default_progress_class
    @default_progress_class ||= ProgressStdout
  end
  
  def self.loop(total_or_description, description = nil, &block)
    description ||= total_or_description
    total = 0 if !total.kind_of?(Numeric)
    
    default_progress_class().new(total, description) do |progress| 
      Kernel.loop do 
        block.call(progress)
      end
    end.value
  end
  
  def self.run(total_or_description, description = nil, &block)
    description ||= total_or_description
    total = 0 if !total.kind_of?(Numeric)
    
    default_progress_class().new(total, description, &block).value
  end
  
  def self.each(array, description = nil, &block)
    default_progress_class().new(array.length, description) do |progress|
      array.each do |e|
        yield e
        progress << 1
      end
    end.value
  end
  
  def self.collect!(array, description = nil, &block)
    default_progress_class().new(array.length, description) do |progress|
      array.collect! do |e|
        result = yield e
        progress << 1
        result
      end
    end.value
  end
  
  def << (progress_amount)
    @progress += progress_amount
  end

  def finished?
    @finished == true
  end
  
  def progress
    @progress
  end

  def percent
    (progress() / @total * 100.0)
  end
  
  def quantifiable?
    !@total.nil? && @total != 0
  end
  
  def value
    @value
  end
  
  def to_s
    message = if quantifiable?
      '%.2f%%' % percent()
    else
      "Wait..."
    end
    
    if @verbose
      message << " (#{@description})"
    end
    
    message
  end
end

# Progress messages are only shown if the operation takes a certain period of time.
class ProgressDelayed < Progress
  include Abstract

  def self.always_report?
    @always_report
  end
  
  def self.always_report=(b)
    @always_report = b
  end

  def initialize(total, description = nil, verbose = false, period = nil, start_delay = nil, &block)
    period = 4 if !period
    start_delay = 3 if !start_delay
    
    @start_delay = start_delay
    
    super(total, description, verbose, &block)
    
    @thread = nil
    
    # the thread may not be killed if 'break' is called during evaluation of &block, so use 'ensure'
    @start_time = Time.now
    started = false
    
    @thread = SafeThread.new do
      Thread.current.priority = 1
      initiated()
      sleep @start_delay

      started = true
      
      show_initiation_message()
      
      while !quantifiable? || progress() <= @total do
        show_progress_message()
        sleep period
      end
    end
      
    @value = yield_to_block(&block)
  end

protected
  def cleanup
    duration = Time.now - @start_time
    
    if duration > @start_delay || self.class.always_report?
      show_conclusion_message(duration)
    end
    
    concluded()
    
    @thread.kill if @thread
  end
  
  def concluded
    @finished = true
  end

  def initiated
  end
    
  def show_conclusion_message(duration)
    abstract
  end

  def show_initiation_message
    abstract
  end
  
  def show_progress_message
    abstract
  end
  
  def yield_to_block(&block)
    begin
      yield self
    ensure
      cleanup()
    end
  end
end

class ProgressDummy < Progress
  def initialize(total = nil, description = nil, verbose = false, &block)
    super
    yield self
  end
end

class ProgressStdout < ProgressDelayed
  def self.output=(stream)
    @output = stream
  end

  def self.output
    @output || $stderr
  end

protected
  def show_initiation_message()
    self.class.output().puts "Please wait#{%Q{: #{@description}} if @description}..."
  end
  
  def show_conclusion_message(duration_secs)
    self.class.output().puts "Done: #{@description} (#{duration_secs} secs)"
  end
  
  def show_progress_message
    self.class.output().puts(to_s())
  end
end
