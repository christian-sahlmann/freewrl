#!/bin/sh
#
# $Id: autogen.sh,v 1.10 2009/06/10 09:39:51 couannette Exp $
#

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
case $(uname -s) in
    Darwin) 
	echo "Mac system: default target is x11"
	echo "(Carbon is not yet supported by this build system)"
	echo "Add this to the configure command line:"
	echo "--with-target=x11"
	echo
	target=x11
	;;
esac

my_options="--with-fontsdir=$fontsdir --with-target=$target $*"

echo
echo "configure options: $my_options"
echo

autoreconf --force --install && \
./configure $my_options
