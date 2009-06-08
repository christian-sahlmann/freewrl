#!/bin/sh
#
# $Id: check_sources.sh,v 1.2 2009/06/08 11:46:08 couannette Exp $
#
# Helper for automake new comers.
# All sources files that are mandatory
# for the build should be referenced in
# Makefile.am or Makefile.sources
#

echo_r() {
    echo "\033[0;31m"$*"\033[0m"
}

echo_y() {
    echo "\033[0;33m"$*"\033[0m"
}

# Identify relative path in source tree
ad=$(grep ^abs_top_srcdir Makefile | awk -F= '{print $2}' | xargs)
dir=$(pwd | sed -e "s|${ad}/||g")

# Prepare the ignore list

# Generated files: we always have this
gen=internal_version.c
# Conditionaly included files
case "$dir" in
    "src/lib") cdi="display_x11.c
display_motif.c
display_aqua.c
ui/fwBareWindow.c
ui/fwMotifWindow.c"
	;;
	"src/libeai") cdi=""
	;;
    "src/bin") cdi=""
	;;
    "src/message") cdi=""
	;;
    "src/plugin") cdi=""
	;;
    "src/sound") cdi=""
	;;
    *) echo_r "Error: please go in any src/\* directory to check source files"
	exit 1
	;;
esac

# Make temporary files for listings
s1=$(mktemp)
s2=$(mktemp)

# Extract source file list from Makefiles
cat ./Makefile.sources | \
    tr -d '\t' | \
    grep -v "^#" | \
    sed -e 's/.* = //g' \
        -e 's/\\[ \t]*$//g' \
        -e '/^[ \t]*$/d' | sort > $s1

# List all source files present
find . -type f -name "*.[hc]" -printf "%P \n" | \
    grep -v 'CVS/' | \
    sort > $s2

# Output the diff
diff -EbB -e $s1 $s2

# Search for required files not added to CVS
# TODO !

# Explanations
echo
echo_y "Warning: some source files are missing in Makefile.source"
echo_y "         they may need to be included in the build system."
echo
echo_y "But ignore:"
echo "$gen"
echo_y "which is generated, and:"
echo "$cdi"
echo_y "which are conditionaly included in Makefile.am."

rm -f $s1 $s2




