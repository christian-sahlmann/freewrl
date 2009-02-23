dnl Configure-time check for a helper application
dnl
dnl Each instance defines a --with-[name] option in
dnl the resulting configure script.
dnl
dnl Usage:
dnl AC_MY_HELPERAPP(name, variable, description, default, if-not-found)
dnl
dnl where:
dnl
dnl name		name for --with flag
dnl variable		variable to set
dnl description		name describing the helper application (ie: 'internet browser')
dnl default		default application to check for in the PATH
dnl if-not-found	what to do if no app was specified and the default was not found
dnl
dnl
dnl Note - a full-path default value can be specified by assigning
dnl the value to VARIABLE beforehand.
dnl ie:
dnl 	BROWSER=/usr/bin/firefox
dnl	AC_MY_HELPERAPP([browser],[BROWSER],[internet browser],[],[AC_MSG_ERROR([fail])]
dnl
AC_DEFUN([AC_MY_HELPERAPP], [
    AC_ARG_WITH([$1],
        [AS_HELP_STRING([--with-$1=PATH],[Path to $3 (default: $4)])],
        [if test "x$withval" = "xno"; then
                $2=false
		m4_if([$5],[],[],[$5])
        else
                $2="$withval"
        fi],[ AC_PATH_PROG([$2], [$4], [false]) ])
    if test "x${$2}" = "xfalse"; then
	m4_if([$5],[],[],[$5])
    else
        AC_MSG_NOTICE([Using ${$2} for $3])
        if test ! -f "${$2}"; then
                AC_MSG_WARN([${$2} not found on this system])
        fi
    fi
    AC_SUBST([$2])
    AC_DEFINE_UNQUOTED($2, "${$2}", [Path to $3])
])
