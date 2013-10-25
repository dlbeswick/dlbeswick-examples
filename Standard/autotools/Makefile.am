ACLOCAL_AMFLAGS = -I m4

src=@abs_srcdir@/../src/Standard
src_platform=@abs_srcdir@/../src/$(standard_platform)/Standard
dxsdk=/c/dxsdk

pkglib_LTLIBRARIES = libstandard.la
libstandard_la_SOURCES = \
	$(src)/Base.cpp \
	$(src)/binstream.cpp \
	$(src)/CMemCheck.cpp \
	$(src)/Collide.cpp \
	$(src)/CommandLine.cpp \
	$(src)/Config.cpp \
	$(src)/ConfigService.cpp \
	$(src)/CriticalSectionBlock.cpp \
	$(src)/Delegate.cpp \
	$(src)/EmbeddedPtr.cpp \
	$(src)/Exception.cpp \
	$(src)/ExceptionFilesystem.cpp \
	$(src)/ExceptionStream.cpp \
	$(src)/FileMgr.cpp \
	$(src)/Filesystem.cpp \
	$(src)/Help.cpp \
	$(src)/Image.cpp \
	$(src)/Log.cpp \
	$(src)/Math.cpp \
	$(src)/MultiDelegate.cpp \
	$(src)/Path.cpp \
	$(src)/pch.cpp \
	$(src)/Profiler.cpp \
	$(src)/PtrGC.cpp \
	$(src)/PtrGCBase.cpp \
	$(src)/PtrGCHost.cpp \
	$(src)/Rand.cpp \
	$(src)/Rect.cpp \
	$(src)/Registrar.cpp \
	$(src)/Ruby.cpp \
	$(src)/StateMachine.cpp \
	$(src)/stdafx.cpp \
	$(src)/SteppableThreadHost.cpp \
	$(src)/Streamable.cpp \
	$(src)/textstream.cpp \
	$(src)/TGA.cpp \
	$(src)/ThreadBase.cpp \
	$(src)/Exception/ExceptionConfig.cpp \
	$(src)/Exception/TextStreamParseException.cpp \
	$(src_platform)/AsyncEvent.cpp \
	$(src_platform)/CriticalSection.cpp \
	$(src_platform)/MemoryMappedFile.cpp \
	$(src_platform)/PlatformThread.cpp \
	$(src_platform)/Timer.cpp
##  $(src_platform)/ThreadStepper.cpp
##  $(src_platform)/DebugInfo.cpp \
##  $(src_platform)/ExceptionWinAPI.cpp \
##  ../src/OSX/Standard/DebugInfo.cpp \
##  ../src/OSX/Standard/DeviceNotification.cpp \
##  ../src/OSX/Standard/Timer.cpp \
##  $(src_platform)/DeviceNotification.cpp \
##  $(src_platform)/DXInput.cpp \

#libstandard_la_LDFLAGS = -no-install
##libstandard_la_LDFLAGS = -ldinput.lib -no-install

libstandard_la_CXXFLAGS = \
	-std=c++0x \
	-I$(src)/.. \
	-I$(src_platform)/..

##	-I$(dxsdk)/Include