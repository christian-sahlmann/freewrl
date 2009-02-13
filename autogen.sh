#!/bin/sh

try_dash=1

if [ ! -z "$1" ] ; then
	shell="$1"
	shift
	if [ ! -x "$shell" ] ; then
		echo "Given shell argument ($shell) is not usable."
		exit 1
	fi
	if [ "$shell" != "/bin/dash" ] ; then
		try_dash=0
	fi
fi

# echo "try_dash=$try_dash"

if [ $try_dash -eq 1 ] ; then
	if [ -x /bin/dash ] ; then
		shell=/bin/dash
	else
		echo "We would have tried /bin/dash but it is not usable."
		exit 1
	fi
fi

# Font directory
# default:
fontsdir=/usr/X11/lib/X11/fonts/TTF
case "$1" in
    debian) fontsdir=/usr/share/fonts/truetype/ttf-bitstream-vera; shift ;;
esac

# Target
case $(uname -s) in
	Darwin) echo "Default target on Mac is x11 (Carbon is not yet supported by this build system)"
			echo "Add this to the command line:"
			echo "--with-target=x11"
			echo;;
esac

my_options="--with-fontsdir=$fontsdir $*"

echo "We will use $shell as the configure shell... "
if [ $try_dash -eq 1 ] ; then
	echo "if this don't work please type: $0 /bin/bash"
fi
echo
echo "configure options: $my_options"
echo
echo "(press ENTER to continue)"
echo
read DUMMY

autoreconf --force --install

CONFIG_SHELL=$shell $shell ./configure $my_options


