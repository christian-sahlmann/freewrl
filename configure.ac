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
AC_INIT([freex3d], [2.0], [freewrl@crc.ca])

# Little cutomizatin: all autotools derived files go in a sub-directory.
AC_CONFIG_AUX_DIR([config])
# If you have custom m4 files, declare this macro (and its twins in Makefile.am)
AC_CONFIG_MACRO_DIR([m4])

# For system detection
AC_CANONICAL_HOST

# automake startup
AM_INIT_AUTOMAKE([subdir-objects]) # this option tells automake to generate
# object files in the same subdir as the source file - this prevent us to have
# all object files in the subdir where Makefile resides...

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
# Common headers the FreeX3D uses
# ===============================

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([ctype.h dirent.h errno.h fcntl.h getopt.h math.h signal.h sched.h stdbool.h stdio.h stdint.h stdlib.h string.h sys/ioctl.h sys/ipc.h sys/msg.h sys/socket.h sys/stat.h sys/time.h sys/types.h sys/wait.h time.h unistd.h])

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
AC_CHECK_FUNCS([strchr strrchr getopt getopt_long getopt_long_only])


# ====================================================
# FreeX3D specific configuration: target system
# ====================================================

# For cross-compilation (to be implemented later)
# we'll use AC_CANONICAL_SYSTEM and target_* variables
# Now we just use the host_* variables
case "$host_os" in
     mingw*) os_type=win32 ;;
     darwin*) os_type=mac ;;
     *) os_type=unix ;;
esac

# OS conditionals
AM_CONDITIONAL([OS_WIN32],[test "$os_type" = "win32"])
AM_CONDITIONAL([OS_UNIX],[test "$os_type" != "win32"])
AM_CONDITIONAL([OS_LINUX],[test "$host_os" = "linux"])
AM_CONDITIONAL([OS_MAC],[test "$os_type" = "mac"])

# ====================================================
# FreeX3D specific configuration: debug build
# ====================================================

AC_MY_DEFINE_SWITCH([debug],[enable debugging (default=off)],off,_DEBUG)
AM_CONDITIONAL([DEBUG],[test "$sw_debug" != no])

# Fonts configuration
AC_ARG_WITH([fontsdir],
            [AS_HELP_STRING([--with-fontsdir=DIR],
                           [font path @<:@DATADIR/$PACKAGE/fonts@:>@])],
            [fontsdir=$withval],
            [fontsdir=$datadir/$PACKAGE/fonts])

AC_DEFINE_DIR([FONTS_DIR],[fontsdir],[Directory for fonts.])


# Mozilla plugin install dir
AC_ARG_WITH([plugindir],
            AS_HELP_STRING([--with-plugindir=DIR],[browser plugin dest-path @<:@LIBDIR/mozilla/plugins@:>@]),
            [if test x$withval = xno; then
              plugindir="${libdir}/mozilla/plugins"
            else
              plugindir="$withval"
            fi],[with_plugindir="${libdir}/mozilla/plugins"])

AC_DEFINE_DIR([PLUGIN_DIR],[plugindir],[Directory for browser plugin.])

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

AC_MSG_NOTICE([Using $found_JAVASCRIPT_ENGINE for Javascript engine library])

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

# ===================================
# Platform specific: windowing system
# ===================================

AC_ARG_WITH([target],
        [AS_HELP_STRING([--with-target],[Select target windowing system (x11,motif,aqua,w32)])],
        [TARGET=$withval],
        [TARGET=auto])

case "$os_type" in
     win32) # Target available: w32
          TARGET=w32
          AC_MSG_FAILURE([Windows not supported yet.])
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
AC_MSG_CHECKING([TODO: Checking for Aqua headers & libraries...])
fi

# X11
if test "$test_x11" = yes ; then
# X11 base
AC_PATH_X
# X11 extra libraries
AC_PATH_XTRA
# Check all X11 stuff with pkg-config
PKG_CHECK_MODULES(XAW, xproto x11 xt xmu xaw7)

# Check for XFree86/Xorg VMODE extension
AC_CHECK_HEADERS(X11/extensions/xf86vmode.h,
        [ dnl *** If X11/extensions/xf86vmode.h exists...
        AC_CHECK_LIB(Xxf86vm, XF86VidModeQueryExtension,
                [AC_DEFINE(HAVE_XF86_VMODE, 1, [Define if you have the Xxf86vm library.])
                        X_PRE_LIBS="$X_PRE_LIBS -lXxf86vm"
                ],,
                $X_LIBS -lXext -lX11 $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[XFree86 VMODE extension not found, Wine will be built without it]]),
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

AC_MSG_RESULT([Target windowing system selected: $TARGET])

# OpenGL
case "$TARGET" in
     aqua) # OpenGL framework should be installed
        if ! test -d /System/Library/Frameworks/OpenGL.framework ; then
           AC_MSG_ERROR([OpenGL framework not found - Check your installation])
        fi
        # FIXME: Early support for OpenGL in OSX with autoconf... freex3d compiles but does not run...
        GL_CFLAGS=""
        GL_LIBS="-framework AppKit -framework Foundation -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL -lGLU"
        AC_SUBST(GL_CFLAGS)
        AC_SUBST(GL_LIBS)
        ;;
     *)
        AC_FIND_OPENGL
        ;;
esac

# JPEG
AC_CHECK_HEADERS([jpeglib.h])
AC_CHECK_LIB([jpeg],[jpeg_start_decompress],[JPEG_LIBS=-ljpeg])
AC_SUBST(JPEG_LIBS)

# PNG
PKG_CHECK_MODULES(PNG, libpng)

# ======================================
# Internet browser plugin configuration
# ======================================

AC_MY_SWITCH(plugin,[enable building of browser plugin],on)

AC_ARG_WITH([plugintarget],
        [AS_HELP_STRING([--with-plugintarget],[Select target plugin architecture (safari,firefox-mac,linux,win)])],
        [PLUGIN_TARGET=$withval],
        [PLUGIN_TARGET=auto])

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
              if test x$found_MOZILLA_PLUGIN = xno; then
                  AC_MSG_WARN([Unable to find an appropriate browser plugin framework, not building plugin])
                  MAYBE_PLUGIN=
              else
                  # If Mozilla plugin is to be compiled we have to define MOZ_X11
                  MOZILLA_PLUGIN_CFLAGS="$MOZILLA_PLUGIN_CFLAGS -DMOZ_X11"
                  AC_MSG_NOTICE([Using $found_MOZILLA_PLUGIN for Mozilla plugin library])
                  MAYBE_PLUGIN=plugin
              fi
              ;;
  esac
fi
AC_SUBST(MAYBE_PLUGIN)

# FreeX3D helpers programs

case "$os_type" in
     mac) # Mac defaults
          DEFAULT_BROWSER="/Applications/Safari.app/Contents/MacOS/Safari"
          IMAGECONVERT="" 
          ;;
     *)   # UNIX defaults
          DEFAULT_BROWSER="firefox"
          ;;
esac # case $os_type

#FIXME: environment variable BROWSER takes precedent over --with-browser= ???

# FIXME: on Mac OSX I want to specify the absolute path of Safari, and the macro
# AC_PATH_PROG can't handle that...

browser_ok=false

# Environment provided browser
if test -n "$BROWSER" ; then
   if test -x "$BROWSER" ; then
      browser_ok=true
   fi
fi

# Configuration default browser
if test -n "$DEFAULT_BROWSER" ; then
   BROWSER=`which $DEFAULT_BROWSER`
   if test -x "$BROWSER" ; then
      browser_ok=true
   fi
fi

if test "$browser_ok" != true ; then
   AC_MSG_WARN([Default browser will be searched in PATH...])
   AC_ARG_VAR([BROWSER], [Path to Internet browser.])
   AC_PATH_PROG([BROWSER], [$BROWSER], [false])
fi

if test "$BROWSER" = "false"; then
  AC_MSG_WARN([Internet browser not found -- freewrl will not process anchors!])
fi
AC_DEFINE_UNQUOTED(BROWSER, "$BROWSER", [Path to Internet browser.])

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
  AC_MSG_ERROR([wget is required])
fi
AC_DEFINE_UNQUOTED(WGET, "$WGET", [Path to wget (http fetch program).])

AC_ARG_VAR([IMAGECONVERT], [Path to convert (image conversion program).])
AC_PATH_PROG([IMAGECONVERT], [convert], [no])
if test "$IMAGECONVERT" = "no"; then
  AC_MSG_ERROR([convert is required])
fi
AC_DEFINE_UNQUOTED(IMAGECONVERT, "$IMAGECONVERT", [Path to convert (image conversion program).])

# =======
# Targets
# =======

FREEX3D_PROGRAM="freex3d"
FREEX3D_SOUND_MANAGER="freex3d_snd"
FREEX3D_MESSAGE_WRAPPER="freex3d_msg"
case $os_type in
     # FIXME: name of plugin?
     win32) FREEX3D_PLUGIN="plugin_windows.dll" ;;
     mac)   FREEX3D_PLUGIN="npfreex3d.dynlib" ;;
     unix)  FREEX3D_PLUGIN="npfreex3d.so" ;;
esac

AC_SUBST(FREEX3D_PROGRAM)
AC_DEFINE_UNQUOTED(FREEX3D_PROGRAM, "$FREEX3D_PROGRAM", [The FreeX3D program name.])

AC_SUBST(FREEX3D_SOUND_MANAGER)
AC_DEFINE_UNQUOTED(FREEX3D_SOUND_MANAGER, "$FREEX3D_SOUND_MANAGER", [The FreeX3D sound wrapper program name.])

AC_SUBST(FREEX3D_MESSAGE_WRAPPER)
AC_DEFINE_UNQUOTED(FREEX3D_MESSAGE_WRAPPER, "$FREEX3D_MESSAGE_WRAPPER", [The FreeX3D message wrapper program name.])

AC_SUBST(FREEX3D_PLUGIN)
AC_DEFINE_UNQUOTED(FREEX3D_PLUGIN, "$FREEX3D_PLUGIN", [The FreeX3D plugin shared object name.])

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
