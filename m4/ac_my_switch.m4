dnl Configure-time switch with default
dnl
dnl Each switch defines an --enable-FOO and --disable-FOO option in
dnl the resulting configure script.
dnl
dnl Usage:
dnl AC_MY_SWITCH(name, description, default, pos-def, neg-def)
dnl
dnl where:
dnl
dnl name	name of switch; generates --enable-name & --disable-name
dnl		options
dnl description	help string is set to this prefixed by "enable" or
dnl		"disable", whichever is the non-default value
dnl default	either "on" or "off"; specifies default if neither
dnl		--enable-name nor --disable-name is specified
dnl pos-def	a symbol to AC_DEFINE if switch is on (optional)
dnl neg-def	a symbol to AC_DEFINE if switch is off (optional)
dnl
AC_DEFUN([AC_MY_SWITCH], [
    AC_MSG_CHECKING(whether to enable $2)
    AC_ARG_ENABLE(
	$1, 
	ifelse($3, on, 
	    [  --disable-[$1]    disable [$2]],
	    [  --enable-[$1]     enable [$2]]),
	[ if test "$enableval" = yes; then
	    AC_MSG_RESULT(yes)
	    sw_$1=yes
	    ifelse($4, , , AC_DEFINE($4,[1],[$2]))
	else
	    AC_MSG_RESULT(no)
	    sw_$1=no
	fi],
        ifelse($3, on,
	   [ AC_MSG_RESULT(yes)
	     sw_$1=yes
	     ifelse($4, , , AC_DEFINE($4,[1],[$2])) ],
	   [ AC_MSG_RESULT(no)
	     sw_$1=no
	]))])
