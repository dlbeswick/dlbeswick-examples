ACLOCAL_AMFLAGS = -I ../../../../Common/Standard/autotools/src/m4

src=$(dlb_proj_root)/src
platform_src=$(dlb_proj_root)/src/$(platform)/timer

SWIG_SOURCES = $(src)/interface.i
##$(platform_src)/platform_interface.i \

pkglib_LTLIBRARIES = timer.la
timer_la_SOURCES = interface_wrap.cxx \
	$(SWIG_SOURCES)

timer_la_LDFLAGS = $(dlb_configure_libs) $(RUBY_EXT_LDFLAGS) -module -shared -avoid-version -shrext ".so"

timer_la_LIBADD = ../../Standard/autotools/libstandard.la

timer_la_CXXFLAGS = \
	-I$(dlb_standard)/src \
	-I$(dlb_standard)/src/$(standard_platform) \
	$(RUBY_EXT_INC_CPPFLAGS) \
	-std=c++0x \
	$(RUBY_EXT_CPPFLAGS) \
	-Wno-format-security

CLEANFILES = interface_wrap.cxx

interface_wrap.cxx: $(SWIG_SOURCES)
	$(SWIG) $(DEFS) -ruby -o $@ $<