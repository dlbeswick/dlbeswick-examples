ACLOCAL_AMFLAGS = -I ../../../../Common/Standard/autotools/src/m4

src=$(dlb_proj_root)/src/midiinterface
platform_src=$(dlb_proj_root)/src/$(dlb_platform_tolower)/midiinterface

SWIG_SOURCES = $(src)/interface.i
##$(platform_src)/platform_interface.i \

pkglib_LTLIBRARIES = midiinterface.la
midiinterface_la_SOURCES = \
	$(src)/MIDIDevices.cpp \
	interface_wrap.cxx \
	$(SWIG_SOURCES)

if LINUX
midiinterface_la_SOURCES += \
	$(platform_src)/MIDIDeviceInJack.cpp \
	$(platform_src)/MIDIDeviceJack.cpp \
	$(platform_src)/MIDIDeviceOutJack.cpp \
	$(platform_src)/PlatformMIDIDevices.cpp
endif

midiinterface_la_LDFLAGS = $(dlb_configure_libs) $(RUBY_EXT_LDFLAGS) -module -shared -avoid-version -shrext ".so"

midiinterface_la_LIBADD = ../../Standard/autotools/libstandard.la

midiinterface_la_CXXFLAGS = \
	-I$(src)/.. \
	-I$(platform_src)/.. \
	-I$(dlb_standard)/src \
	-I$(dlb_standard)/src/$(standard_platform) \
	$(RUBY_EXT_INC_CPPFLAGS) \
	-std=c++0x \
	$(RUBY_EXT_CPPFLAGS) \
	-Wno-format-security

CLEANFILES = interface_wrap.cxx

interface_wrap.cxx: $(SWIG_SOURCES)
	$(SWIG) $(DEFS) -ruby -o $@ $<