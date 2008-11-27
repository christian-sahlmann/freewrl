dnl Configure-time switch with default
dnl
dnl Each switch defines an --enable-FOO or --disable-FOO option in
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
dnl * Modified by Ian Stakenvicius, 2008-11-21:
dnl * -to handle assignment to --enable-name when it occurs
dnl *  (note, value is thrown away)
dnl * -to use AC_HELP_STRING instead of unformatted help messages
dnl * -to make the code executed on pos-def/neg-def be generic
dnl *  instead of AC_DEFINE
dnl
AC_DEFUN([AC_MY_SWITCH], [
    AC_MSG_CHECKING([whether to $2])
    AC_ARG_ENABLE(
	[$1], 
	m4_if([$3], [on],
	    [AS_HELP_STRING([--disable-$1],[$2])],
	    [AS_HELP_STRING([--enable-$1],[$2])]
	),
	[if test x$enableval = xno; then
	    AC_MSG_RESULT([no])
	    sw_$1=no
	    m4_if([$5],[],[],[$5])
	else
	    AC_MSG_RESULT(yes)
	    sw_$1=yes
	    m4_if([$4],[],[],[$4])
	fi],
        m4_if([$3], [on], [
	    AC_MSG_RESULT([no])
	    sw_$1=no
	    m4_if([$4],[],[],[$4])
        ],[
	    AC_MSG_RESULT(no)
	    sw_$1=no
	    m4_if([$5],[],[],[$5])
	])
    )
])
dnl
dnl ===========================================================
dnl AC_MY_DEFINE_SWITCH
dnl * Wrapper of AC_MY_SWITCH that performs an AC_DEFINE
dnl
AC_DEFUN([AC_MY_DEFINE_SWITCH],[
    AC_MY_SWITCH(
	[$1],
	[$2],
	[$3],
	m4_if([$4],[],[],
	[AC_DEFINE([$4],[1],[$2])]),
	m4_if([$5],[],[],
	[AC_DEFINE([$5],[1],[$2])]))
])
