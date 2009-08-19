#!/bin/sh
#
# $Id: autogen.sh,v 1.13 2009/08/19 22:20:07 couannette Exp $
#

# options
eai=1
curl=0
debug=0
trace=0

# Pick up autogen options and leave other arguments to configure
while getopts "cdet" option
do
  case $option in
      c ) curl=1
	  ;;
      d ) debug=1
	  ;;
      e ) eai=0
	  ;;
      t ) trace=1
	  ;;
  esac
done
shift $(($OPTIND - 1))


platform=$(uname -s)

# Font directory
# default:
fontsdir=/usr/X11/lib/X11/fonts/TTF

if [ -f /etc/debian_version ] ; then
    if [ -d /usr/share/fonts/truetype/ttf-bitstream-vera ] ; then
	fontsdir=/usr/share/fonts/truetype/ttf-bitstream-vera
	echo "Fonts dir: $fontsdir"
    else
	echo "Debian system: please install the ttf-bitstream-vera font package."
    fi
else
    echo "Default fonts dir: $fontsdir"
fi

# Target
target=motif

case $platform in
    Darwin) 
	echo "Mac system: default target is x11"
	echo "(Carbon is not yet supported by this build system)"
	echo "Add this to the configure command line:"
	echo "--with-target=x11"
	echo
	target=x11

	port=$(port version)
	if [ $? -eq 0 ] ; then
	    # we have Mac Ports installed
	    add_path=/opt/local
	fi
	;;
    win32|CYGWIN*|cygwin*)
	echo "Windows platform detected : $platform"
	target=win32
	;;
esac

if [ ! -z "$add_path" ] ; then
    cflags="CPPFLAGS=-I$add_path/include"
    lflags="LDFLAGS=-L$add_path/lib"
fi

my_options="--with-fontsdir=$fontsdir --with-target=$target"

echo
echo "configure options: $my_options"
echo

if [ $curl -eq 1 ] ; then
    my_options="$my_options --enable-libcurl"
else
    my_options="$my_options --disable-libcurl"
fi

if [ $debug -eq 1 ] ; then
    my_options="$my_options --enable-debug"
else
    my_options="$my_options --disable-debug"
fi

if [ $eai -eq 1 ] ; then
    my_options="$my_options --enable-libeai"
else
    my_options="$my_options --disable-libeai"
fi

if [ $trace -eq 1 ] ; then
    cflags="$cflags -DTEXVERBOSE"
fi

[ ! -e configure ] && autoreconf --force --install

./configure $my_options $cflags $lflags "$@"
