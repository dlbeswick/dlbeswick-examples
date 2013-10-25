AC_PREREQ(2.63)

define([_EMAIL], [dlbeswick@gmail.com])

AC_INIT([DJ], [0.1], _EMAIL)
AC_CONFIG_AUX_DIR([.])

AM_INIT_AUTOMAKE([1.9 foreign])
AC_CONFIG_FILES([Makefile])

AC_LANG([C++])

AC_CONFIG_MACRO_DIR([m4])

m4_include([../../../Common/Standard/autotools/src/dlb-standard.ac])

DLB_CONFIG_SUBDIR([Standard/autotools])
DLB_CONFIG_SUBDIR([dsound/autotools])
DLB_CONFIG_SUBDIR([timer/autotools])
DLB_CONFIG_SUBDIR([mediatag/autotools])
DLB_CONFIG_SUBDIR([midiinterface/autotools])

AC_OUTPUT
