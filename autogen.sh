#!/bin/sh
#
# $Id: autogen.sh,v 1.17 2009/10/30 15:53:48 couannette Exp $
#

# options
eai=1
curl=0
debug=0
trace=0
plugin=1
motif=1

# variables
cflags=
lflags=

# constants
default_fontsdir=/usr/X11/lib/X11/fonts/TTF
default_target=motif

# Pick up autogen options and leave other arguments to configure
while getopts ":cdetpmh" option
do
  case $option in
      c ) curl=1
	  echo "Enabling libcurl support."
	  ;;
      d ) debug=1
	  echo "Enabling debug mode."
	  ;;
      e ) eai=0
	  echo "Disabling libeai."
	  ;;
      t ) trace=1
	  echo "Enabling traces."
	  ;;
	  p ) plugin=0
	  echo "Disabling plugin."
	  ;;
	  m ) motif=0
	  echo "Disabling Motif."
	  ;;
      h ) # Help
	  cat <<EOF 
Example invocation of autogen.sh on Mac

 $ ./autogen.sh -edpm -- --without-glew

Disabling libeai.
Enabling debug mode.
Disabling plugin.
Disabling Motif.
Remaining args: --without-glew
Platform: Mac
Mac system: default target is x11 (maybe motif)
(Carbon is not yet supported by this build system)
Regenerating configure files...

EOF
	  exit 0
	  ;;
  esac
done
shift $(($OPTIND - 1))

echo "Remaining args: $@" # must be preceded by '--'

platform=$(uname -s)

case $platform in
    Linux)
	echo "Platform: Linux"

# 	cflags="$cflags -D_GNU_SOURCE"

	if [ -f /etc/debian_version ] ; then
	    echo "Distribution: Debian / Ubuntu"
	    if [ -d /usr/share/fonts/truetype/ttf-bitstream-vera ] ; then
		fontsdir=/usr/share/fonts/truetype/ttf-bitstream-vera
	    fi
	else
	    echo "Please install the ttf-bitstream-vera font package."
	    exit 0
	fi

	case $motif in
		0) target=x11;;
		1) target=motif;;
	esac
	;;

    Darwin)
	echo "Platform: Mac"

	fontsdir=$default_fontsdir

	echo "Mac system: default target is x11 (maybe motif)"
	echo "(Carbon is not yet supported by this build system)"

	case $motif in
		0) target=x11;;
		1) target=motif;;
	esac

	port=$(port version)
	if [ $? -eq 0 ] ; then
	    # we have Mac Ports installed
	    add_path=/opt/local
	fi
	;;

    win32|CYGWIN*|cygwin*)
	echo "Platform: Windows ($platform)"

	fontsdir="C:\\Windows\\Fonts"
	target=win32
	;;    
esac

if [ ! -z "$add_path" ] ; then
    cflags="$cflags -I$add_path/include"
    lflags="$lflags -L$add_path/lib"
fi

if [ $debug -eq 1 ] ; then
    my_options="$my_options --enable-debug"
else
    my_options="$my_options --disable-debug"
fi

if [ $curl -eq 1 ] ; then
    my_options="$my_options --enable-libcurl"
else
    my_options="$my_options --disable-libcurl"
fi

if [ $eai -eq 1 ] ; then
    my_options="$my_options --enable-libeai"
else
    my_options="$my_options --disable-libeai"
fi

if [ $plugin -eq 1 ] ; then
    my_options="$my_options --enable-plugin"
else
    my_options="$my_options --disable-plugin"
fi

if [ $trace -eq 1 ] ; then
    cflags="$cflags -DVERBOSE"
fi

# Strip useless spaces in cflags & lflags
cf=$(echo $cflags|xargs)
lf=$(echo $lflags|xargs)

echo "Regenerating configure files..."
autoreconf --force --install

my_options="$my_options --with-fontsdir=$fontsdir --with-target=$target CFLAGS=$cf LDFLAGS=$lf $@"

echo "Configure options are: $my_options"
echo "Starting configure..."

./configure $my_options
