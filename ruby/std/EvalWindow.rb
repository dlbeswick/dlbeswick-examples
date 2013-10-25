require 'std/PersistentWindow'

class EvalWindowBase < PersistentWindow
  def initialize(owner)
    super(owner, 'Eval')
  end

private
  def evalCommand(str)
    begin
      [Object.module_eval(str).to_s, true]
    rescue Exception=>e
      result = "Error during evaluation:\n#{e.to_s}"
      $stderr.puts result
      [result, false]
    end
  end
end

class EvalWindowWx < EvalWindowBase
  include PlatformGUISpecific

  def make_controls
  	super
  	
  	makeEntryField(clientArea())
  	makeResultField(clientArea())
  	clientArea().layout
  end

	def makeEntryField(parent)
		v = Wx::TextValidator.new
		class << v
			def on_char(event)
				if event.key_code == WXK_ENTER || event.key_code == WXK_NUMPAD_ENTER
					event.skip
				end
			end
		end
	
		text = Wx::TextCtrl.new(parent, -1, '', Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE, Wx::TE_MULTILINE | Wx::TE_PROCESS_ENTER, v)
		parent.sizer.add(text, 1, Wx::GROW)
		
		text.evt_char do |event|
			if event.key_code == Wx::K_RETURN || event.key_code == Wx::K_NUMPAD_ENTER
				result = evalCommand(text.value)
				
				@resultText.value = result[0]
				
				text.set_selection(-1, -1) if result[1]
			else
				event.skip
			end
		end
	end
  
	def makeResultField(parent)
		@resultText = Wx::TextCtrl.new(parent, -1, '', Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE, Wx::TE_MULTILINE | Wx::HSCROLL)
		parent.sizer.add(@resultText, 4, Wx::GROW)
	end
end

class EvalWindowFox < EvalWindowBase
  include PlatformGUISpecific

  def initialize(owner)
		super
		
		recalc
    create
    show
	end
	
	def make_controls
		super
		
		FXVerticalFrame.new(self) do |frame|
			frame.layoutHints = LAYOUT_FILL
			frame.create
			
			FXSpring.new(frame, LAYOUT_FILL, 0, 30) do |ctrl|
				ctrl.create
				makeEntryField(ctrl)
			end
	
			FXSpring.new(frame, LAYOUT_FILL, 0, 70) do |ctrl|
				ctrl.create
				makeResultField(ctrl)
			end
		end
	end
	
	def makeEntryField(parent)
		FXText.new(parent) do |ctrl|
			ctrl.layoutHints = LAYOUT_FILL
			ctrl.connect(SEL_KEYPRESS) do |sender, sel, data|
				if data.code == KEY_Return || data.code == KEY_KP_Enter 
					if data.state == 0
						@resultText.setText(evalCommand(ctrl.text.chomp))
						ctrl.selectAll
						1
					else
						0
					end
				else
					0
				end
			end
			ctrl.create
		end
	end

	def makeResultField(parent)
		@resultText = FXText.new(parent, nil, 0, TEXT_WORDWRAP) do |ctrl|
			ctrl.layoutHints = LAYOUT_FILL
			ctrl.create
		end
	end
end
