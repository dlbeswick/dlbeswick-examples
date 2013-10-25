ACLOCAL_AMFLAGS = -I ../../../../Common/Standard/autotools/src/m4

dsound_src=$(dsound)/src/dsound
dsound_platform_src=$(dsound)/src/$(platform)/dsound

SWIG_SOURCES = $(dsound_platform_src)/platform_interface.i \
	$(dsound_src)/interface.i

pkglib_LTLIBRARIES = dsound.la
dsound_la_SOURCES = $(dsound_src)/BufferModifier.cpp \
	$(dsound_src)/DecoderSox.cpp \
	$(dsound_src)/ModifierNode.cpp \
	$(dsound_src)/MP3Decoder.cpp \
	$(dsound_src)/MP3Encoder.cpp \
	$(dsound_src)/MP3EncoderLame.cpp \
	$(dsound_src)/pch.cpp \
	$(dsound_src)/SoundBuffer.cpp \
	$(dsound_src)/SoundBufferMedia.cpp \
	$(dsound_src)/SoundConsumer.cpp \
	$(dsound_src)/SoundDevice.cpp \
	$(dsound_src)/SoundGenerator.cpp \
	$(dsound_src)/SoundGeneratorOutput.cpp \
	$(dsound_src)/SoundLibrary.cpp \
	$(dsound_src)/SoundModifier.cpp \
	$(dsound_src)/SoundVisualisation.cpp \
	$(dsound_src)/SoundVisualisationProgressive.cpp \
	$(dsound_src)/TranscoderFlac.cpp \
	$(dsound_src)/TranscoderSox.cpp \
	interface_wrap.cxx \
	$(SWIG_SOURCES)

if USE_FFT
#dsound_la_SOURCES += $(dsound_src)/FFT.cpp 
endif

dsound_la_LDFLAGS = $(dlb_configure_libs) $(RUBY_EXT_LDFLAGS) -module -shared -avoid-version -shrext ".so"
#-dlopen ../../Standard/autotools/libstandard.la

if LINUX
dsound_la_SOURCES += \
	$(dsound_platform_src)/ExceptionPortAudio.cpp \
	$(dsound_platform_src)/PortAudioDevice.cpp \
	$(dsound_platform_src)/PortAudioLibrary.cpp

#dsound_la_LDFLAGS += -lmad -lmp3lame -lid3tag -lsox -lportaudio
else
#dsound_la_LDFLAGS += -module -o dsound.so -ldxguid -ldxerr9 -ldsound -lmad -lmp3lame -lid3tag -dll
endif

##dsound_la_LIBADD = ../../../Common/Standard/autotools/build/libstandard.la
dsound_la_LIBADD = ../../Standard/autotools/libstandard.la

dsound_la_CXXFLAGS = \
	-I$(dsound_src)/.. \
	-I$(dsound_platform_src)/.. \
	-I$(standard)/src \
	-I$(standard)/src/$(standard_platform) \
	$(RUBY_EXT_INC_CPPFLAGS) \
	-std=c++0x \
	$(RUBY_EXT_CPPFLAGS) \
	-Wno-format-security

##	-I$(dxsdk)/Include \

CLEANFILES = interface_wrap.cxx

interface_wrap.cxx: $(SWIG_SOURCES) $(dsound_src)/capabilities.h
	$(SWIG) $(DEFS) -ruby -o $@ $<