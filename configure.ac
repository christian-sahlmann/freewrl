#
# FreeX3D new build system
#
# Michel Briand - 2008, with a lot of help from automake@gnu.org.
#
# Special thanks to Ralf Wildenhues.
#
# Distributed under the GNU GPL.
#

# Autoconf startup, we define the package's name, and version: 
# but this is not the program's version, nor the library's version.
# We also define the maintainer address.
AC_INIT([freex3d], [2.0], [michelbriand@free.fr])

# Little cutomizatin: all autotools derived files go in a sub-directory.
AC_CONFIG_AUX_DIR([config])
# If you have custom m4 files, declare this macro (and its twins in Makefile.am)
AC_CONFIG_MACRO_DIR([m4])

# automake startup
AM_INIT_AUTOMAKE

# We have a script (vtempl) that uses a system command (sed): we need
# to be sure to call the good sed binary whatever the system is. So we 
# ask autotools to check for us the good sed command (please look into
# vtempl.in for details):
AC_CONFIG_FILES([vtempl],[chmod +x vtempl])

# Create config.h with all C defines that have been checked by configure
# (definitions like PACKAGE_NAME and conditionals like HAVE_STDLIB_H).
AC_CONFIG_HEADER([config.h])

# Before libtool 2.2 we need to disable C++ and Fortran explicitely:
# C++ is not used, so related checks are not needed
m4_defun([_LT_AC_LANG_CXX_CONFIG], [:])
# Same line for Fortran
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])

# Include all that's required to build a library
AC_PROG_LIBTOOL

# ===============================
# Common headers the FreeX3D uses
# ===============================

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h stdio.h stdlib.h string.h sys/ioctl.h unistd.h sys/stat.h sys/ipc.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIGNAL
AC_CHECK_TYPE([sighandler_t])
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strrchr])

# ====================================================
# FreeX3D specific configuration: see doc/DEPENDENCIES
# ====================================================

# Threads
ACX_PTHREAD

# FreeType (font rendering)
PKG_CHECK_MODULES(FREETYPE, freetype2 >= 9.3.0)

# Javascript engine
# check order:
# xulrunner 1.9 (provides mozilla-js.pc, from the libmozjs-dev package in testing)
# xulrunner 1.8 (provides xulrunner-js.pc, from the libmozjs-dev package in stable)
# firefox 2.0 
# firefox 1.5

AC_MSG_NOTICE([Determining Javascript engine to build against])

JAVASCRIPT_ENGINE=""

AC_ARG_WITH([javascript-engine],
	[AS_HELP_STRING([--with-javascript-engine=PATH],
		[Search Javascript engine development files in PATH.])],
	[js_engine_fallback_path=$withval],
	[js_engine_fallback_path=/usr])

PKG_CHECK_MODULES( JAVASCRIPT_ENGINE_XULRUNNER_19,
				   mozilla-js >= 1.9,
				   [JAVASCRIPT_ENGINE="xulrunner19"],[

	AC_MSG_WARN([mozilla-js not found])
	PKG_CHECK_MODULES( JAVASCRIPT_ENGINE_XULRUNNER_18,
  					   xulrunner-js >= 1.8,
					   [JAVASCRIPT_ENGINE="xulrunner18"],[

		AC_MSG_WARN([xulrunner-js not found])
		PKG_CHECK_MODULES( JAVASCRIPT_ENGINE_SEAMONKEY,
  					   	   seamonkey-js,
					   	   [JAVASCRIPT_ENGINE="seamonkey"],[
				AC_MSG_WARN([seamonkey-js not found])
				PKG_CHECK_MODULES( JAVASCRIPT_ENGINE_FIREFOX,
								   firefox-js,
								   [JAVASCRIPT_ENGINE="firefox"],[

				AC_MSG_WARN([firefox-js not found])
				AC_MSG_FAILURE([None Javascript engine were found. Configuration cannot continue.])

dnl I tried this below, but it fails because of libmozjs.so own dependencies...
dnl --> the check header works, but the check lib cannot be specified in a generic form
dnl     so it will fails
dnl
dnl 		CPPFLAGS_save="$CPPFLAGS"
dnl 		CPPFLAGS="$CPPFLAGS -I$js_engine_fallback_path/include -DXP_UNIX -DJS_THREADSAFE"
dnl 		AC_CHECK_HEADERS([mozjs/jsapi.h],[],
dnl 				[AC_MSG_FAILURE([Javascript engine not found, cannot continue.])])
dnl 		LDFLAGS_save="$LDFLAGS"
dnl 		LDFLAGS="$LDFLAGS -L$js_engine_fallback_path/lib -L/usr/lib/xulrunner-devel-1.9/lib -lplds4 -lplc4 -lnspr4 -lpthread -ldl"
dnl 		AC_CHECK_LIB([mozjs],[JS_NewRuntime],[],
dnl 				[AC_MSG_FAILURE([Javascript engine not found, cannot continue.])]
dnl 				["-L$js_engine_fallback_path/lib -lplds4 -lplc4 -lnspr4 -lpthread -ldl"])
dnl 		LDFLAGS="$LDFLAGS_save"
dnl 		CPPFLAGS="$CPPFLAGS_save"

])])])])

case $JAVASCRIPT_ENGINE in
	 xulrunner19)
		JAVASCRIPT_ENGINE_CFLAGS="$JAVASCRIPT_ENGINE_XULRUNNER_19_CFLAGS"
		JAVASCRIPT_ENGINE_LIBS="$JAVASCRIPT_ENGINE_XULRUNNER_19_LIBS"
		;;
	 xulrunner18)
		JAVASCRIPT_ENGINE_CFLAGS="$JAVASCRIPT_ENGINE_XULRUNNER_18_CFLAGS"
		JAVASCRIPT_ENGINE_LIBS="$JAVASCRIPT_ENGINE_XULRUNNER_18_LIBS"
		;;
	 fallback)
		JAVASCRIPT_ENGINE_CFLAGS="$JAVASCRIPT_ENGINE_FALLBACK_CFLAGS"
		JAVASCRIPT_ENGINE_LIBS="$JAVASCRIPT_ENGINE_FALLBACK_LIBS"
		;;
	 *)
esac
AC_SUBST(JAVASCRIPT_ENGINE)
AC_SUBST(JAVASCRIPT_ENGINE_CFLAGS)
AC_SUBST(JAVASCRIPT_ENGINE_LIBS)

AC_MSG_NOTICE([Using $JAVASCRIPT_ENGINE for Javascript engine library])


# XML
PKG_CHECK_MODULES(XML, libxml-2.0)

# Expat
EXPAT_LIB=""
AC_ARG_WITH(expat, [ --with-expat=<dir> Use expat from here],
                      [
                      expat=$withval
                      CPPFLAGS="$CPPFLAGS -I$withval/include"
                      LDFLAGS="$LDFLAGS -L$withval/lib"
                      ]
                      )
AC_CHECK_HEADERS(expat.h, [AC_DEFINE(HAVE_EXPAT_H)],
     [AC_MSG_ERROR([Can't find expat.h. Please install expat.])])
AC_CHECK_LIB(expat, XML_ParserCreate, [EXPAT_LIBS="-lexpat"],
   [AC_MSG_ERROR([Can't find expat library. Please install expat.])])
AC_SUBST(EXPAT_LIBS)

# X11 base
AC_PATH_X

# X11 extra libraries
AC_PATH_XTRA

# Check all X11 stuff with pkg-config
PKG_CHECK_MODULES(XAW, xproto x11 xt xmu xaw7)

# X11 Motif
AC_FIND_MOTIF
AM_CONDITIONAL([WANT_MOTIF], [test $with_motif != no])

# OpenGL
AC_FIND_OPENGL

# JPEG
AC_CHECK_HEADERS([jpeglib.h])
AC_CHECK_LIB([jpeg],[jpeg_start_decompress],[JPEG_LIBS=-ljpeg])
AC_SUBST(JPEG_LIBS)

# PNG
PKG_CHECK_MODULES(PNG, libpng)

# Mozilla plugin devel
AC_MSG_NOTICE([Determining browser plug-in framework to build against])
PKG_CHECK_MODULES(MOZILLA_PLUGIN, iceape-plugin, [
  PLUGENG="iceape"
],[
  AC_MSG_WARN([iceape-plugin not found])
  PKG_CHECK_MODULES(MOZILLA_PLUGIN, xulrunner-plugin, [
    PLUGENG="xulrunner"
  ],[
    AC_MSG_WARN([xulrunner-plugin not found])
    PKG_CHECK_MODULES(MOZILLA_PLUGIN, seamonkey-plugin, [
      PLUGENG="seamonkey"
    ],[
      AC_MSG_WARN([seamonkey-plugin not found])
      PKG_CHECK_MODULES(MOZILLA_PLUGIN, firefox-plugin, [
        PLUGENG="firefox"
      ],[
        AC_MSG_WARN([firefox-plugin not found])
        AC_MSG_ERROR([Unable to find an appropriate browser plug-in framework])
      ])
    ])
  ])
])
# If Mozilla plugin is to be compiled we have to define MOZ_X11
if ! test -z "${MOZILLA_PLUGIN_CFLAGS}" ; then
   MOZILLA_PLUGIN_CFLAGS="$MOZILLA_PLUGIN_CFLAGS -DMOZ_X11"
fi

# FreeX3D help programs
AC_ARG_VAR([SOUNDCONV], [path to SOUNDCONV])
AC_PATH_PROG([SOUNDCONV], [sox], [no])
if test "$SOUNDCONV" = "no"; then
  AC_MSG_ERROR([sox is required])
fi
AC_DEFINE_UNQUOTED(SOUNDCONV, "$SOUNDCONV", [Path to sox (sound conversion program).])

AC_ARG_VAR([UNZIP], [path to UNZIP])
AC_PATH_PROG([UNZIP], [unzip], [no])
if test "$UNZIP" = "no"; then
  AC_MSG_ERROR([unzip is required])
fi
AC_DEFINE_UNQUOTED(UNZIP, "$UNZIP", [Path to unzip ((de-)compression program).])


AC_CONFIG_FILES([Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/libFreeX3D.pc
src/plugin/Makefile
src/sound/Makefile
src/message/Makefile])

AC_OUTPUT
