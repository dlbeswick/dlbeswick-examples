require 'Config'
require 'rexml/document'

module ConfigXML
	include DLB::Config

	raise "This module has broken Config's contract."

	def self.included(receiver)
		receiver.extend ClassMethods
	end

	module ClassMethods
		include DLB::Config::ClassMethods
		
		def from_element(element)
			newObj = eval("#{element.attributes['class']}.new")
			newObj.from_element(element)
			newObj
		end

		def load(path=nil)
			xml = openConfigXML(path)
			from_element(self.xmlElement(xml))
		end
	end

	def ConfigXML.configExtension
		"xml"
	end
	
	def load(path=nil)
		xml = openConfigXML(path)
		raise "No XML config file exists for load." if !xml.root
		from_element(self.xmlElement(xml))
	end
	
	def save(path=nil)
		xml = openConfigXML(path)
		xml.elements.delete_all('*')

		e = REXML::Element.new("Config")
		write_element(e)

		xml << e

		file = configPath.to_file('w')
		file.write(xml)
		file.close
	end

	def from_element(xmlElement)
		xmlElement.elements.each do |e|
			eval("@#{e.name} = #{e.attributes['class']}.from_element(e)")
		end
	end
	
	def write_element(element)
		begin
			element.attributes["class"] = self.class.name
		
			id = configId
			element.attributes["id"] = id if id
		
			custom_write_element(element)
			write_element_variables(element)
		rescue Exception => e
			puts "Exception writing class of type #{self.class.name} id #{self.object_id}: #{e}"
		end
	end
	
protected
	def custom_write_element(element)
	end
	
	def openConfigXML(path)
		path = configPath if !path
		
		xml = nil
		begin
			#puts "Opening #{path}..."
			xml = REXML::Document.new path.to_file
			#puts "XML opened."
		rescue
			xml = REXML::Document.new
		rescue Exception=>e
	 		raise "Couldn't open config file #{configPath} (#{e})"
		end
 		
		xml
	end

	def write_element_variables(element)
		configVariables.each do |v|
			var = eval(v.to_s)

			if var
				begin
					varElement = REXML::Element.new(v.to_s[/[^@]+/])

					var.write_element(varElement)

					element << varElement
				rescue Exception => e
					puts "Exception writing element variable of type #{var.class.name} id #{var.object_id}: #{e}"
					raise
				end
			end
		end
	end
	
	def xmlElement(xml)
		xml.elements["Config"]
	end
end

module ConfigXMLEnumerable
	#include ConfigXML
	# ruby bug: include will not work for predefined modules, so we must duplicate code

	def write_element(element)
		element.attributes["class"] = self.class.name

		each do |i|
			if i
				varElement = REXML::Element.new("Item")
				i.write_element(varElement)
				element << varElement
			end
		end
	end
end

module ConfigXMLStandardOutput
	def write_element(element)
		element.attributes['class'] = self.class.name
		element.text=to_s
	end
end

class Array
	include ConfigXMLEnumerable

	def self.from_element(element)
		c = []

		element.elements.each do |e|
			c << eval("#{e.attributes['class']}.from_element(e)")
		end

		c
	end
end

class Hash
	include ConfigXMLEnumerable

	def self.from_element(element)
		c = {}
		
		element.elements.each do |e|
			key = eval("#{e.elements[0].attributes['class']}.from_element(e.elements[0])")
			val = eval("#{e.elements[1].attributes['class']}.from_element(e.elements[1])")
			c[key] = val
		end
		
		c
	end
end

class String
	include ConfigXMLStandardOutput

	def self.from_element(element)
		element.text
	end
end

class Fixnum
	include ConfigXMLStandardOutput

	def self.from_element(element)
		element.text.to_i
	end
end

class Numeric
	include ConfigXMLStandardOutput

	def self.from_element(element)
		element.text.to_f
	end
end

class TrueClass
	include ConfigXMLStandardOutput

	def self.from_element(element)
		true
	end
end

class FalseClass
	include ConfigXMLStandardOutput

	def self.from_element(element)
		false
	end
end

class Path
	include ConfigXML
end