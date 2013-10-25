AC_PREREQ(2.63)

define([_EMAIL], [dlbeswick@gmail.com])
define([_ROOT], [../../../..])
define([dlb_ac_standard_autotools_dir], [Standard/autotools])

AC_INIT([dsound], [0.1], _EMAIL)
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_MACRO_DIR([../../../../Common/Standard/autotools/src/m4])

AM_INIT_AUTOMAKE([1.9 foreign])
AC_LANG([C++])
AC_PROG_CXX

AX_SWIG_ENABLE_CXX
AX_RUBY_EXT

LT_INIT([dlopen],[disable-static])

m4_include(_ROOT[/Common/Standard/autotools/src/dlb-standard.ac])

AC_CONFIG_FILES([Makefile])

AM_CONDITIONAL([USE_FFT], [test])

#tbd: windows directsound
#AC_CHECK_HEADER([dsound.h],[],[AC_MSG_ERROR([cannot find dsound.h, set CPPFLAGS=-Ipath])])

#AC_ARG_WITH([sox], [AS_HELP_STRING([--with-sox])])
#AC_ARG_WITH([flac], [AS_HELP_STRING([--with-flac])])

DLB_REQUIRE_LIB([sox])
DLB_REQUIRE_LIB([mad])
DLB_REQUIRE_LIB([portaudio])
DLB_REQUIRE_LIB([mp3lame])
DLB_REQUIRE_LIB([id3tag])

AC_DEFINE([WITH_FLAC],[WITH_SOX])

AC_SUBST([ROOT],_ROOT)
AC_SUBST([dsound],[\${abs_srcdir}/..])
AC_SUBST([standard],[\${abs_srcdir}/${ROOT}/Common/Standard])
AC_SUBST([standard_autotools_dir],dlb_ac_standard_autotools_dir)
#dxsdk=/c/dxsdk

AC_OUTPUT

