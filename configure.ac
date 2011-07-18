#
# FreeWRL new build system
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
AC_INIT([freewrl], [1.22.12], [freewrl-help@lists.sourceforge.net])

# Little customization: all autotools derived files go in a sub-directory.
AC_CONFIG_AUX_DIR([config])
# If you have custom m4 files, declare this macro (and its twins in Makefile.am)
AC_CONFIG_MACRO_DIR([m4])

# For system detection
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# automake startup
AM_INIT_AUTOMAKE([subdir-objects]) # this option tells automake to generate
# object files in the same subdir as the source file - this prevent us to have
# all object files in the subdir where Makefile resides...

# "shaved feature" of automake 1.11... output only compiled file, not gcc command line
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

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

# Hack to pick up pkgconfig definition files in our source directory:
export PKG_CONFIG_PATH=$srcdir:$PKG_CONFIG_PATH

# ===============================
# Common headers the FreeWRL uses
# ===============================

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([ctype.h dirent.h errno.h fcntl.h getopt.h math.h signal.h sched.h stdarg.h stdbool.h stdio.h stdint.h stdlib.h string.h sys/ioctl.h sys/ipc.h sys/msg.h sys/socket.h sys/stat.h sys/time.h sys/types.h sys/wait.h time.h unistd.h limits.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIGNAL
AC_CHECK_TYPE([sighandler_t])
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
dnl AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strrchr strdup strndup strerror getopt getopt_long getopt_long_only malloc calloc realloc usleep gettimeofday])


# ====================================================
# FreeWRL specific configuration: target system
# ====================================================

# For cross-compilation (to be implemented later)
# we'll use AC_CANONICAL_SYSTEM and target_* variables
# Now we just use the host_* variables
case "$host_os" in
     *mingw32*|*cygwin*) os_type=win32 ;;
     darwin*) os_type=mac ;;
     *) os_type=linux ;;
esac

echo "Platform: $host_os, $os_type"

# OS conditionals
AM_CONDITIONAL([OS_WIN32],[test "$os_type" = "win32"])
AM_CONDITIONAL([OS_UNIX],[test "$os_type" != "win32"])
AM_CONDITIONAL([OS_LINUX],[test "$os_type" = "linux"])
AM_CONDITIONAL([OS_MAC],[test "$os_type" = "mac"])

# ====================================================
# FreeWRL specific configuration: debug build
# ====================================================

# A lot of DEBUG, _DEBUG, DEBUG_ defines are out there. So I prefer to create
# a new one for FreeWRL:

AC_MY_DEFINE_SWITCH([debug],[enable debugging (default=off)],off,FW_DEBUG)
AM_CONDITIONAL([FW_DEBUG],[test "$sw_debug" != no])

# debug feature: colorized threads
AC_MY_DEFINE_SWITCH([thread_colorized],[enable colorized thread console output (default=off)],off,FREEWRL_THREAD_COLORIZED)

# debug feature: plugin debug messages
AC_MY_DEFINE_SWITCH([debug_plugin],[enable debugging (default=off)],off,FW_DEBUG_PLUGIN)

# Fonts configuration
AC_ARG_WITH([fontsdir],
            [AS_HELP_STRING([--with-fontsdir=DIR],
                           [font path @<:@DATADIR/$PACKAGE/fonts@:>@])],
            [fontsdir=$withval],
            [fontsdir=$datadir/$PACKAGE/fonts])

AC_DEFINE_DIR([FONTS_DIR],[fontsdir],[Directory for fonts.])

# ====================================================
# FreeWRL specific configuration: library checking
# ====================================================
# zlib
ZLIB_LIBS=-lz
AC_ARG_WITH(zlib,AS_HELP_STRING([--with-zlib=<dir>],[Use zlib from here]),[
                zlib=$withval
                ZLIB_CFLAGS="-I$withval/include"
                ZLIB_LIBS="-L$withval/lib $ZLIB_LIBS"])

  # save global cflags/libs
  oCFLAGS=$CFLAGS;oLDFLAGS=$LDFLAGS;oCPPFLAGS=$CPPFLAGS
  # set cflags/libs for this case
  CPPFLAGS="$CPPFLAGS $ZLIB_CFLAGS"
  LDFLAGS="$LDFLAGS $ZLIB_LIBS"

AC_CHECK_HEADERS(zlib.h, [AC_DEFINE(HAVE_ZLIB_H)],
     [AC_MSG_ERROR([Can't find zlib.h. Please install zlib.])])
AC_CHECK_LIB(z, deflate, [],
   [AC_MSG_ERROR([Can't find zlib library. Please install zlib.])])

  # restore global cflags/libs
  CPPFLAGS=$oCPPFLAGS;LDFLAGS=$oLDFLAGS;CFLAGS=$oCFLAGS

AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LIBS)

# workaround LIBS appended with -lz, how to prevent this ???
LIBS=

# Threads
ACX_PTHREAD

# Fix for proper use of --as-needed
# http://udrepper.livejournal.com/19395.html
PTHREAD_CFLAGS=$(echo $PTHREAD_CFLAGS|sed -e 's/-pthreads\{0,1\}//')
PTHREAD_LIBS="$PTHREAD_LIBS -lpthread"

# FreeType (font rendering)
PKG_CHECK_MODULES(FREETYPE, freetype2)

# Fontconfig [optional]
AC_MY_SWITCH(fontconfig,[enable fontconfig support],off)
if test x$sw_fontconfig = xyes; then
    PKG_CHECK_MODULES(FONTCONFIG, fontconfig, AC_DEFINE(HAVE_FONTCONFIG, 1, [Define if you have fontconfig installed.]))
fi


# libcurl [optional]
AC_MY_SWITCH(libcurl,[enable libcurl (could be used instead of wget)],off)
if test x$sw_libcurl = xno; then
   :
else
   PKG_CHECK_MODULES(LIBCURL, libcurl, AC_DEFINE(HAVE_LIBCURL, 1, [Define if you have the libcurl library.]))
fi


# Javascript engine
# check order:
# xulrunner 1.9 (provides mozilla-js.pc, from the libmozjs-dev package in testing)
# xulrunner 1.8 (provides xulrunner-js.pc, from the libmozjs-dev package in stable)
# seamonkey (firefox-2.0 equivalent packaging)
# firefox (2.0 or 1.5)
# firefox (firefox-2.0 possible alternate packaging)

AC_MSG_NOTICE([Determining Javascript engine to build against])
found_JAVASCRIPT_ENGINE=no
case "$os_type" in
     mac) 
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,spidermonkey,1.7.0)
        ;;
     *)
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,mozilla-js,1.9)
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,xulrunner-js,1.8)
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,seamonkey-js)
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,firefox-js)
        AC_MY_CHECK_MODULE(JAVASCRIPT_ENGINE,firefox2-js)
        ;;
esac

## if no package yet, test for spidermonkey or other 'js' library
if test x$found_JAVASCRIPT_ENGINE = xno; then
  AC_MSG_ERROR([Unable to find an appropriate javascript engine])
fi

## Provide a fix for include path for systems with erroneous mozilla-js.pc
if test x$found_JAVASCRIPT_ENGINE = xmozilla-js; then
  LIBMOZJS_INCDIR=`$PKG_CONFIG --variable=includedir mozilla-js 2>/dev/null`
  if (echo $JAVASCRIPT_ENGINE_CFLAGS | egrep '(un)?stable') > /dev/null 2>&1; then
    if test -f $LIBMOZJS_INCDIR/unstable/jsapi.h; then
      AC_MSG_NOTICE([Discovered jsapi.h exists in unstable includedir])
      JAVASCRIPT_ENGINE_CFLAGS="${JAVASCRIPT_ENGINE_CFLAGS/unstable/stable} -DMOZILLA_JS_UNSTABLE_INCLUDES"
      AC_SUBST(JAVASCRIPT_ENGINE_CFLAGS)
    fi
  fi
fi

AC_MSG_NOTICE([Using $found_JAVASCRIPT_ENGINE for Javascript engine library])

# XML
PKG_CHECK_MODULES(XML, libxml-2.0)

# Expat
EXPAT_LIBS=-lexpat
AC_ARG_WITH(expat,AS_HELP_STRING([--with-expat=<dir>],[Use expat from here]),[
		if test "x$withval" != xno ; then
	                with_expat=yes
	                EXPAT_CFLAGS="-I$withval/include"
	                EXPAT_LIBS="-L$withval/lib $EXPAT_LIBS"
		else
			AC_MSG_NOTICE([expat support disabled, checking for libxml2 headers instead])
			with_expat=no
		fi],[with_expat=check])

# save global cflags/libs
oCFLAGS=$CFLAGS;oLDFLAGS=$LDFLAGS;oCPPFLAGS=$CPPFLAGS;oLIBS=$LIBS

if test "x$with_expat" != xno ; then
  # set cflags/libs for this case
  CPPFLAGS="$CPPFLAGS $EXPAT_CFLAGS"
  LDFLAGS="$LDFLAGS $EXPAT_LIBS"

  AC_CHECK_HEADERS(expat.h, [AC_DEFINE(HAVE_EXPAT_H)],
     [AC_MSG_ERROR([Can't find expat.h. Please install expat.])])
  AC_CHECK_LIB(expat, XML_ParserCreate, [],
     [AC_MSG_ERROR([Can't find expat library. Please install expat.])])
else
  # not using expat so set these to be empty
  EXPAT_LIBS=
  EXPAT_CFLAGS=
  # set cflags/libs for this case
  CFLAGS="$CFLAGS $XML_CFLAGS"
  CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
  LDFLAGS="$LDFLAGS $XML_LIBS"
  # find libxml/parser.h and set define accordingly
  AC_CHECK_HEADERS(libxml/parser.h,[AC_DEFINE(HAVE_LIBXML_PARSER_H)],[AC_MSG_ERROR([Could not find headers for libxml2])])
fi

# restore global cflags/libs
CPPFLAGS=$oCPPFLAGS;LDFLAGS=$oLDFLAGS;CFLAGS=$oCFLAGS;LIBS=$oLIBS

AC_SUBST(EXPAT_CFLAGS)
AC_SUBST(EXPAT_LIBS)


# ===================================
# Platform specific: windowing system
# ===================================

AC_ARG_WITH([target],
        [AS_HELP_STRING([--with-target],[Select target windowing system (x11,motif,aqua,win32)])],
        [TARGET=$withval],
        [TARGET=auto])

case "$os_type" in
     win32) # Target available: win32
          case "$TARGET" in
               auto) TARGET=win32 ;;
               x11|win32) : ;;
               *) AC_MSG_FAILURE([Platform is $os_type, target available are: win32,x11.]) ;;
          esac
          ;;
     mac) # Target available: aqua,x11 (and possibly Motif in the future)
          case "$TARGET" in
               auto) TARGET=aqua ;;
               x11|aqua) : ;;
               *) AC_MSG_FAILURE([Platform is $os_type, target available are: aqua,x11.]) ;;
          esac
          ;;
     *) # Target available: x11,motif
          case "$TARGET" in
               auto) TARGET=x11 ;;
               x11|motif) : ;;
               *) AC_MSG_FAILURE([Platform is $os_type, target available are: x11,motif.]) ;;
          esac
          ;;
esac

test_x11=no
test_aqua=no

case "$TARGET" in
     aqua) test_aqua=yes ;;
     x11|motif) test_x11=yes ;;
esac

# Aqua
if test "$test_aqua" = yes ; then
AC_MSG_WARN([TODO: Checking for Aqua headers & libraries...])
fi

# X11
if test "$test_x11" = yes ; then
# X11 base
AC_PATH_X
# X11 extra libraries
AC_PATH_XTRA
# Check all X11 stuff with pkg-config
case "$os_type" in
     mac) PKG_CHECK_MODULES(XAW, xproto x11 xt xmu) ;;
     *)   PKG_CHECK_MODULES(XAW, xproto x11 xt xmu xaw7) ;;
esac

# Check for XFree86/Xorg VMODE extension
AC_CHECK_HEADERS(X11/extensions/xf86vmode.h,
        [ dnl *** If X11/extensions/xf86vmode.h exists...
        AC_CHECK_LIB(Xxf86vm, XF86VidModeQueryExtension,
                [AC_DEFINE(HAVE_XF86_VMODE, 1, [Define if you have the Xxf86vm library.])
                        X_PRE_LIBS="$X_PRE_LIBS -lXxf86vm"
                ],,
                $X_LIBS -lXext -lX11 $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[XFree86 VMODE extension not found, this package will be built without it]]),
        [#include <X11/Xlib.h>])
fi

if test "$TARGET" = motif ; then
# X11 Motif
AC_FIND_MOTIF
fi

# ================================
# Platform conditional compilation
# ================================

# automake conditional must not be declared conditionaly ;)
AM_CONDITIONAL([WINDOW_USE_AQUA], [test "$TARGET" = aqua])
AM_CONDITIONAL([WINDOW_USE_X11], [test "$TARGET" = x11])
AM_CONDITIONAL([WINDOW_USE_MOTIF], [test "$TARGET" = motif])
AM_CONDITIONAL([WINDOW_USE_WIN32], [test "$TARGET" = win32])

AC_MSG_RESULT([Target windowing system selected: $TARGET])

# OpenGL
case "$TARGET" in
     aqua) # OpenGL framework should be installed
        if ! test -d /System/Library/Frameworks/OpenGL.framework ; then
           AC_MSG_ERROR([OpenGL framework not found - Check your installation])
        fi
        # FIXME: Early support for OpenGL in OSX with autoconf... freewrl compiles but does not run...
        GL_CFLAGS=""
        GL_LIBS="-framework AppKit -framework Foundation -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL -lGLU"
        AC_SUBST(GL_CFLAGS)
        AC_SUBST(GL_LIBS)
        ;;
     win32) # Currently we have it in default search path (w32api)
        # set by env vars
	#GL_CFLAGS=""
	#GL_LIBS=""
        AC_SUBST(GL_CFLAGS)
        AC_SUBST(GL_LIBS)
	;;
     *)
        AC_FIND_OPENGL
        ;;
esac

# Glew
AC_ARG_WITH([glew],AS_HELP_STRING([--with-glew=<dir>],[Use glew from here]),[],[with_glew=check])

case "$with_glew" in
     no) # Do not use GLEW
	AC_MSG_NOTICE([glew disabled.])
	;;
     *)  # Search for GLEW
	if test "x$with_glew" = xcheck; then
		# do nothing, glew must be found in the regular path
		GLEW_CFLAGS=""
		GLEW_LIBS="-lGLEW"
	else
	if test "x$with_glew" = xyes; then
		# do nothing, glew must be found in the regular path
		GLEW_CFLAGS=""
		GLEW_LIBS="-lGLEW"
	else
		# assign the path specified via --with-glew
                GLEW_CFLAGS="-I$with_glew/include"
                GLEW_LIBS="-L$with_glew/lib -lGLEW"
	fi; fi

	# save global cflags/libs
	oCFLAGS=$CFLAGS;oLDFLAGS=$LDFLAGS;oCPPFLAGS=$CPPFLAGS;oLIBS=$LIBS
	# set cflags/libs for this case
	CPPFLAGS="$CPPFLAGS $GLEW_CFLAGS $GL_CFLAGS"
	LDFLAGS="$LDFLAGS $GLEW_LIBS $GL_LIBS"
	AC_CHECK_HEADERS([GL/glew.h], [
		AC_DEFINE([HAVE_GLEW_H], [], [We have the glew library.])
		AC_CHECK_LIB([GLEW], [glewInit], [],
     	  		[AC_MSG_ERROR([glew headers found but glew library not found. Please correct this.])])
		],[AC_MSG_RESULT([glew not found. FreeWRL will not use it.])])

	# restore global cflags/libs
	CPPFLAGS=$oCPPFLAGS;LDFLAGS=$oLDFLAGS;CFLAGS=$oCFLAGS;LIBS=$oLIBS
	AC_SUBST(GLEW_CFLAGS)
	AC_SUBST(GLEW_LIBS)
	;;
esac

# JPEG
AC_ARG_WITH(jpeg,AS_HELP_STRING([--with-jpeg=<dir>],[Use jpeg from here]),[],[with_jpeg=check])
case "$with_jpeg" in
    no)
	AC_MSG_ERROR([libjpeg is a required dependency])
	;;
    yes|check)
	JPEG_CFLAGS=""
	JPEG_LIBS="-ljpeg"
	;;
    *)
	JPEG_CFLAGS="-I$with_jpeg/include"
	JPEG_LIBS="-L$with_jpeg/lib -ljpeg"
	;;
esac
# save global cflags/libs
oCFLAGS=$CFLAGS;oLDFLAGS=$LDFLAGS;oCPPFLAGS=$CPPFLAGS;oLIBS=$LIBS
# set cflags/libs for this case
CPPFLAGS="$CPPFLAGS $JPEG_CFLAGS"
LDFLAGS="$LDFLAGS $JPEG_LIBS"

AC_CHECK_HEADERS([jpeglib.h], [AC_DEFINE(HAVE_JPEGLIB_H)],[
     # workaround failed test under mingw
     AC_TRY_CPP([
#include <stdio.h>
#undef PACKAGE
#undef VERSION
#include <jpeglib.h>],[jpeg_ok=yes],[jpeg_ok=no])
     AC_MSG_RESULT([$jpeg_ok])
     if test x$jpeg_ok = xno; then
	AC_MSG_ERROR([libjpeg headers are required])
     fi])
AC_CHECK_LIB([jpeg],[jpeg_start_decompress],[],[
     # workaround failed test under mingw
     AC_TRY_LINK([
#include <stdio.h>
#include "jpeglib.h"], 
		 [jpeg_read_raw_data(NULL, NULL, 0)], 
		 [AC_MSG_RESULT(found)],
		 [AC_MSG_RESULT(not found)
		  AC_MSG_ERROR(Couldn't find jpeg stuff in ${withval})])
     ])
# restore global cflags/libs
CPPFLAGS=$oCPPFLAGS;LDFLAGS=$oLDFLAGS;CFLAGS=$oCFLAGS;LIBS=$oLIBS
AC_SUBST(JPEG_CFLAGS)
AC_SUBST(JPEG_LIBS)

# PNG
PKG_CHECK_MODULES(PNG, libpng)

# Imlib2
PKG_CHECK_MODULES(IMLIB2, imlib2)

# ======================================
# LibFreeWRLEAI configuration
# ======================================

AC_MY_SWITCH(libeai,[enable building of libEAI],off)
if test x$sw_libeai = xno; then
  MAYBE_LIBEAI=
else
  MAYBE_LIBEAI=libeai
fi
AC_SUBST(MAYBE_LIBEAI)

# ======================================
# Sound module configuration
# ======================================

AC_MY_SWITCH(sound,[enable building of sound module],on)
if test x$sw_sound = xno; then
  AC_MSG_NOTICE([Not building sound module])
  MAYBE_SOUND=
else
  MAYBE_SOUND=sound
fi
# do AC_SUBST(MAYBE_SOUND) after checking for existance of soundconv

# ======================================
# Internet browser plugin configuration
# ======================================

AC_MY_SWITCH(plugin,[enable building of browser plugin],on)

AC_ARG_WITH([plugintarget],
        [AS_HELP_STRING([--with-plugintarget],[Select target plugin architecture (safari,firefox-mac,linux,win)])],
        [PLUGIN_TARGET=$withval],
        [PLUGIN_TARGET=auto])

# Mozilla plugin install dir
AC_ARG_WITH([plugindir],
            AS_HELP_STRING([--with-plugindir=DIR],[browser plugin install path @<:@/usr/lib/mozilla/plugins@:>@]),
            [plugindir=$withval],			       # user supplied value
            [plugindir=$libdir/mozilla/plugins])	       # default value

AC_DEFINE_DIR([PLUGIN_DIR],[plugindir],[Directory where to install browser plugin.])

if test x$sw_plugin = xno; then
  AC_MSG_NOTICE([Not building browser plug-in])
  MAYBE_PLUGIN=
else

  case "$PLUGIN_TARGET" in
      safari) # Build plugin for Safari
              AC_MSG_FAILURE([Build system does not currently support this plugin target.])
              MAYBE_PLUGIN=
              ;;

      firefox-mac) # Build plugin for Firefox on the Mac
              AC_MSG_FAILURE([Build system does not currently support this plugin target.])
              MAYBE_PLUGIN=
              ;;

      *)      # Build plugin for Netscape compatible browser on Linux
              AC_MSG_NOTICE([Determining browser plug-in framework to build against])
              found_MOZILLA_PLUGIN=no
              AC_MY_CHECK_MODULE(MOZILLA_PLUGIN,iceape-plugin)
              AC_MY_CHECK_MODULE(MOZILLA_PLUGIN,xulrunner-plugin)
              AC_MY_CHECK_MODULE(MOZILLA_PLUGIN,seamonkey-plugin)
              AC_MY_CHECK_MODULE(MOZILLA_PLUGIN,firefox-plugin)
              AC_MY_CHECK_MODULE(MOZILLA_PLUGIN,mozilla-plugin)
              if test x$found_MOZILLA_PLUGIN = xno; then
                  AC_MSG_WARN([Unable to find an appropriate browser plugin framework, not building plugin])
                  MAYBE_PLUGIN=
              else
                  # If Mozilla plugin is to be compiled we have to define MOZ_X11
                  MOZILLA_PLUGIN_CFLAGS="$MOZILLA_PLUGIN_CFLAGS -DMOZ_X11"
                  AC_MSG_NOTICE([Using $found_MOZILLA_PLUGIN for Mozilla plugin library])

                  # XEmbed indicators use PRBool type from nspr, check for support here
                  PKG_CHECK_MODULES([NSPR],[nspr],[
                    AC_DEFINE([HAVE_NSPR], [1], [Define if you have nspr installed.])
                    MOZILLA_PLUGIN_CFLAGS="$MOZILLA_PLUGIN_CFLAGS $NSPR_CFLAGS"
                  ],[])
		  
                  MAYBE_PLUGIN=plugin
                  AC_MSG_NOTICE([Installing plugin to $plugindir])

		  # Check for JRIEnv type to see if OJI can be defined
		  AC_CHECK_TYPE(JRIEnv,[AC_DEFINE(OJI,1,[Use Original Java Interface within plugin])],,$MOZILLA_PLUGIN_CFLAGS)
              fi
              ;;
  esac

fi
AC_SUBST(MAYBE_PLUGIN)

PLUGIN_CFLAGS=$MOZILLA_PLUGIN_CFLAGS
PLUGIN_LIBS=
AC_SUBST(PLUGIN_CFLAGS)
AC_SUBST(PLUGIN_LIBS)

#
# Check if 'int32' type is defined (would be included in plugin's includes)

  # save global cflags/libs
  oCPPFLAGS=$CPPFLAGS;oLDFLAGS=$LDFLAGS;oCFLAGS=$CFLAGS
  CFLAGS="$CFLAGS $PLUGIN_CFLAGS"
AC_CHECK_TYPES([int32, uint32, int16, uint16], [], [], [[#include <npapi.h>]])
  # restore global cflags/libs
  CPPFLAGS=$oCPPFLAGS;LDFLAGS=$oLDFLAGS;CFLAGS=$oCFLAGS


# ======================================
# Status bar configuration
# ======================================
AC_ARG_WITH([statusbar],
        [AS_HELP_STRING([--with-statusbar=STATUSBAR],[Select status bar flavour -- 'hud' or 'standard'])],
        [statusbar="$withval"],
        [statusbar="standard"])

case "$statusbar" in
     standard|hud) AC_MSG_NOTICE([Status bar flavor: $statusbar]) ;; # valid input
     *) AC_MSG_ERROR([Invalid input for status bar flavor: $statusbar]) ;;
esac
AM_CONDITIONAL([STATUSBAR_STD], [test "$statusbar" = standard])
AM_CONDITIONAL([STATUSBAR_HUD], [test "$statusbar" = hud])

# ======================================
# OSC enable
# ======================================

AC_MY_SWITCH(osc,[enable building of OSC plugin support],off)
if test x$sw_osc = xyes; then
    PKG_CHECK_MODULES(LIBLO, liblo, 
        AC_DEFINE([WANT_OSC],[1],
                  [Define if liblo exists and OSC support wanted]),
                  AC_MSG_ERROR([Skipping OSC. No liblo installed.]))
fi

# ======================================
# Experimental 2D overlay
# ======================================
AC_MY_SWITCH(twodee,[enable building of 2D overlay support],off)
if test x$sw_twodee = xyes; then
	AC_DEFINE(WANT_2D_OVER,1,[Install 2D overlay])
fi

# ======================================
# FreeWRL helpers programs
# ======================================

#
# Defaults:
# if we want to set a "hard" (unchecked) default, specify the target variable (ie: BROWSER )
# if we want to specify the program but let AC_PATH_PROG find the path, specify DEFAULT_[target] (ie: DEFAULT_BROWSER )
case "$os_type" in
     mac) # Mac defaults
          DEFAULT_BROWSER=
          BROWSER="/Applications/Safari.app/Contents/MacOS/Safari"
          DEFAULT_SOUNDCONV=sox
          DEFAULT_UNZIP=unzip
          DEFAULT_WGET=wget
          DEFAULT_IMAGECONVERT=convert
          DEFAULT_JAVAC=javac
          DEFAULT_JAR=jar
          ;;
     win32) # Windows defaults
          DEFAULT_BROWSER=
	  BROWSER='C:\Program Files\Mozilla Firefox\firefox.exe'
          DEFAULT_SOUNDCONV=sox.exe
          DEFAULT_UNZIP=unzip.exe
          DEFAULT_WGET=wget.exe
          DEFAULT_IMAGECONVERT=convert.exe
          DEFAULT_JAVAC=javac.exe
          DEFAULT_JAR=jar.exe
          ;;
     *)   # UNIX defaults
          DEFAULT_BROWSER=firefox
          DEFAULT_SOUNDCONV=sox
          DEFAULT_UNZIP=unzip
          DEFAULT_WGET=wget
          DEFAULT_IMAGECONVERT=convert
          DEFAULT_JAVAC=javac
          DEFAULT_JAR=jar
          ;;
esac # case $os_type

# ======================================
# Java configuration
# ======================================

AC_MY_DEFINE_SWITCH([java],[enable Java support (default=off)],off,JAVA_SUPPORT)
if test x$sw_java = xno; then
  AC_MSG_NOTICE([Not building java classes])
  MAYBE_JAVA=
else
  # Java classes install dir configuration
  AC_ARG_WITH([javadir],
              [AS_HELP_STRING([--with-javadir=DIR],
              [font path @<:@DATADIR/$PACKAGE/lib@:>@])],
              [fontsdir=$withval],
              [fontsdir=$datadir/$PACKAGE/lib])
  AC_DEFINE_DIR([JAVA_DIR],[javadir],[Directory for java classes.])
  # java compiler check (javac)
  AC_MY_HELPERAPP([javac],[JAVAC],[java compiler],[$DEFAULT_JAVAC],[AC_MSG_ERROR([fail])])
  # java archiver check (jar)
  AC_MY_HELPERAPP([jar],[JAR],[java archiver],[$DEFAULT_JAR],[AC_MSG_ERROR([fail])])
  MAYBE_JAVA=java
fi
AC_SUBST([MAYBE_JAVA])

# Quick hack to merge Doug's patch... we'll have to clean up
# this with proper autotools macros for Windows...

## Browser check
AC_MY_HELPERAPP([browser],[BROWSER],[internet browser],[$DEFAULT_BROWSER],[
        AC_MSG_WARN([Internet browser $DEFAULT_BROWSER not found! FreeWRL will not process anchors unless the BROWSER environment variable is set!])
        ])

## Soundconv program check (sox)
if test x$MAYBE_SOUND = xsound; then
    AC_MY_HELPERAPP([soundconv],[SOUNDCONV],[sound converter],[$DEFAULT_SOUNDCONV],[
        MAYBE_SOUND=
        AC_MSG_WARN([No soundconv helper -- Not building sound module])
        ])
fi
AC_SUBST([MAYBE_SOUND])

## Unzip check
AC_MY_HELPERAPP([unzip],[UNZIP],[zip decompressor],[$DEFAULT_UNZIP],[
        AC_MSG_ERROR([No zip decompressor, please specify one using --with-unzip=PATH])
        ])

## Wget check
AC_MY_HELPERAPP([wget],[WGET],[http fetcher],[$DEFAULT_WGET],[
        AC_MSG_ERROR([No http fetcher, please specify one using --with-wget=PATH])
        ])

## Imageconvert check
AC_MY_HELPERAPP([imageconvert],[IMAGECONVERT],[image converter],[$DEFAULT_IMAGECONVERT],[
        AC_MSG_ERROR([No image converter, please specify one using --with-imageconvert=PATH])
        ])


# =======
# Targets
# =======

FREEWRL_PROGRAM="freewrl"
FREEWRL_SOUND_MANAGER="freewrl_snd"
FREEWRL_MESSAGE_WRAPPER="freewrl_msg"
case $os_type in
     # FIXME: name of plugin?
     win32) FREEWRL_PLUGIN="libFreeWRLplugin.dll" ;;
     mac)   FREEWRL_PLUGIN="libFreeWRLplugin.dynlib" ;;
     *)     FREEWRL_PLUGIN="libFreeWRLplugin.so" ;;
esac

AC_SUBST(FREEWRL_PROGRAM)
AC_DEFINE_UNQUOTED(FREEWRL_PROGRAM, "$FREEWRL_PROGRAM", [The FreeWRL program name.])

AC_SUBST(FREEWRL_SOUND_MANAGER)
AC_DEFINE_UNQUOTED(FREEWRL_SOUND_MANAGER, "$FREEWRL_SOUND_MANAGER", [The FreeWRL sound wrapper program name.])

AC_SUBST(FREEWRL_MESSAGE_WRAPPER)
AC_DEFINE_UNQUOTED(FREEWRL_MESSAGE_WRAPPER, "$FREEWRL_MESSAGE_WRAPPER", [The FreeWRL message wrapper program name.])

AC_SUBST(FREEWRL_PLUGIN)
AC_DEFINE_UNQUOTED(FREEWRL_PLUGIN, "$FREEWRL_PLUGIN", [The FreeWRL plugin shared object name.])

# Final generation
AC_CONFIG_FILES([Makefile
src/Makefile
src/bin/Makefile
src/lib/Makefile
src/lib/libFreeWRL.pc
src/plugin/Makefile
src/libeai/Makefile
src/libeai/libFreeWRLEAI.pc
src/sound/Makefile
src/java/Makefile
src/message/Makefile])

AC_OUTPUT
