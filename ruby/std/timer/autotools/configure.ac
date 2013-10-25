AC_PREREQ(2.63)

define([_EMAIL], [dlbeswick@gmail.com])
define([_ROOT], [../../../..])
AC_CONFIG_MACRO_DIR([../../../../Common/Standard/autotools/src/m4])

AC_INIT([timer], [0.1], _EMAIL)
AC_CONFIG_AUX_DIR([.])

AM_INIT_AUTOMAKE([1.9 foreign])
AC_LANG([C++])

AX_SWIG_ENABLE_CXX
AX_RUBY_EXT

AC_PROG_CXX
LT_INIT([dlopen],[disable-static])

m4_include(_ROOT[/Common/Standard/autotools/src/dlb-standard.ac])

AC_CONFIG_FILES([Makefile])

AC_SUBST([dlb_proj_root],[\${abs_srcdir}/..])
AC_SUBST([dlb_src],[\${dlb_proj_root}/../src])
AC_SUBST([dlb_standard],[\${abs_srcdir}/_ROOT/Common/Standard])
AC_SUBST([dlb_configure_libs])

AC_OUTPUT

