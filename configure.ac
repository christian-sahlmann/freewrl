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

# For system detection
AC_CANONICAL_HOST

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
AC_CHECK_HEADERS([fcntl.h stdio.h stdlib.h string.h sys/ioctl.h unistd.h sys/stat.h sys/socket.h sys/ipc.h signal.h errno.h time.h sys/time.h])

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
# FreeX3D specific configuration: target system
# ====================================================

# For cross-compilation (to be implemented later)
# we'll use AC_CANONICAL_SYSTEM and target_* variables
# Now we just use the host_* variables
case "$host_os" in
     mingw*) os_type=win32 ;;
     darwin) os_type=mac ;;
     *) os_type=unix ;;
esac

# OS conditionals
AM_CONDITIONAL([OS_WIN32],[test "$os_type" = "win32"])
AM_CONDITIONAL([OS_UNIX],[test "$os_type" != "win32"])
AM_CONDITIONAL([OS_LINUX],[test "$host_os" = "linux"])
AM_CONDITIONAL([OS_MAC],[test "$os_type" = "mac"])

# Target conditionals
AC_ARG_WITH([target],
	AS_HELP_STRING([--with-target],[Select target windowing system (x11,motif,aqua,w32)]),
	[TARGET=$withval],[TARGET=auto])

if test "$TARGET" = auto; then
   case "$os_type" in
   	win32) TARGET=win32 ;;
   	mac) TARGET=aqua ;;
	*) TARGET=motif ;;
   esac
fi

AC_MSG_RESULT([Target windowing system selected: $TARGET])

# ====================================================
# FreeX3D specific configuration: debug build
# ====================================================
AC_ARG_ENABLE([debug],
     [  --enable-debug    Turn on debugging],
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
     esac],[debug=false])
     AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_ARG_WITH(fontsdir,
            AC_HELP_STRING([--with-fontsdir=@<:@dir@:>@],
                           [Select <dir> as font path (default=$datadir/$PACKAGE/fonts)]),
            [fontsdir=$withval],[fontsdir=$datadir/$PACKAGE/fonts])

AC_DEFINE_DIR([FONTS_DIR],[fontsdir],[Directory for fonts.])

# ====================================================
# FreeX3D specific configuration: library checking
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
AC_ARG_WITH(expat,AS_HELP_STRING([--with-expat=<dir>],[Use expat from here]),[
		expat=$withval
		CPPFLAGS="$CPPFLAGS -I$withval/include"
		LDFLAGS="$LDFLAGS -L$withval/lib"])

AC_CHECK_HEADERS(expat.h, [AC_DEFINE(HAVE_EXPAT_H)],
     [AC_MSG_ERROR([Can't find expat.h. Please install expat.])])
AC_CHECK_LIB(expat, XML_ParserCreate, [EXPAT_LIBS="-lexpat"],
   [AC_MSG_ERROR([Can't find expat library. Please install expat.])])
AC_SUBST(EXPAT_LIBS)

# Aqua
# test to implement here
AM_CONDITIONAL([WINDOW_USE_AQUA], [test "$TARGET" = aqua])

# X11 base
AC_PATH_X
# X11 extra libraries
AC_PATH_XTRA
# Check all X11 stuff with pkg-config
PKG_CHECK_MODULES(XAW, xproto x11 xt xmu xaw7)
AM_CONDITIONAL([WINDOW_USE_X11], [test "$TARGET" = x11])

AC_CHECK_HEADERS([X11/extensions/xf86vmode.h],
		 [HAVE_XF86_VMODE_H=1; X_EXTRA_LIBS="$X_EXTRA_LIBS -lXxf86vm"],
		 [])

# X11 Motif
AC_FIND_MOTIF
AM_CONDITIONAL([WINDOW_USE_MOTIF], [test "$TARGET" = motif])

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
AC_ARG_VAR([SOUNDCONV], [Path to sox (sound conversion program).])
AC_PATH_PROG([SOUNDCONV], [sox], [no])
if test "$SOUNDCONV" = "no"; then
  AC_MSG_ERROR([sox is required])
fi
AC_DEFINE_UNQUOTED(SOUNDCONV, "$SOUNDCONV", [Path to sox (sound conversion program).])

AC_ARG_VAR([UNZIP], [Path to unzip ((de-)compression program).])
AC_PATH_PROG([UNZIP], [unzip], [no])
if test "$UNZIP" = "no"; then
  AC_MSG_ERROR([unzip is required])
fi
AC_DEFINE_UNQUOTED(UNZIP, "$UNZIP", [Path to unzip ((de-)compression program).])

AC_ARG_VAR([WGET], [Path to wget (http fetch program).])
AC_PATH_PROG([WGET], [wget], [no])
if test "$WGET" = "no"; then
  AC_MSG_ERROR([unzip is required])
fi
AC_DEFINE_UNQUOTED(WGET, "$WGET", [Path to wget (http fetch program).])

AC_ARG_VAR([IMAGECONVERT], [Path to convert (image conversion program).])
AC_PATH_PROG([IMAGECONVERT], [convert], [no])
if test "$IMAGECONVERT" = "no"; then
  AC_MSG_ERROR([unzip is required])
fi
AC_DEFINE_UNQUOTED(IMAGECONVERT, "$WGET", [Path to convert (image conversion program).])


# Final generation
AC_CONFIG_FILES([Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/libFreeX3D.pc
src/plugin/Makefile
src/sound/Makefile
src/message/Makefile])

AC_OUTPUT
