#!/bin/sh
#
# $Id: autogen.sh,v 1.14 2009/10/06 13:16:55 couannette Exp $
#

# options
eai=1
curl=0
debug=0
trace=0

# variables
cflags=
lflags=

# constants
default_fontsdir=/usr/X11/lib/X11/fonts/TTF
default_target=motif

# Pick up autogen options and leave other arguments to configure
while getopts ":cdet" option
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
  esac
done
shift $(($OPTIND - 1))

echo "Remaining args: $@" # must be preceded by '--'

platform=$(uname -s)

case $platform in
    Linux)
	echo "Platform: Linux"
	if [ -f /etc/debian_version ] ; then
	    echo "Distribution: Debian / Ubuntu"
	    if [ -d /usr/share/fonts/truetype/ttf-bitstream-vera ] ; then
		fontsdir=/usr/share/fonts/truetype/ttf-bitstream-vera
	    fi
	else
	    echo "Please install the ttf-bitstream-vera font package."
	    exit 0
	fi
	;;

    Darwin)
	echo "Platform: Mac"

	fontsdir=$default_fontsdir

	echo "Mac system: default target is x11"
	echo "(Carbon is not yet supported by this build system)"
	target=x11

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

my_options="--with-fontsdir=$fontsdir --with-target=$target"

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
    cflags="$cflags -DVERBOSE"
fi

# Strip useless spaces in cflags & lflags
cf=$(echo $cflags|xargs)
lf=$(echo $lflags|xargs)

echo "Regenerating configure files..."
autoreconf --force --install

my_options="$my_options CFLAGS=\"$cf\" LDFLAGS=\"$lf\" $@"

echo "Configure options are: $my_options"
echo "Starting configure..."

./configure "$my_options"
