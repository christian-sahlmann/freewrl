dnl AC_MY_CHECK_MODULE
dnl Usage:  AC_MY_CHECK_MODULE([variable],[module-name],[min-version])
dnl
dnl   variable    = The environment variable to write to 
dnl                 (parameter 1 of PKG_CHECK_MODULES)
dnl
dnl   module-name = The name of the module to check for 
dnl                 (parameter 2 of PKG_CHECK_MODULES)
dnl
dnl   min-version = (optional) version number to check, 
dnl                 check is for greater-than-or-equal-to
dnl
dnl This function runs as long as $found_[variable] = "no", so please 
dnl include the line 'found_[variable]=no' before using this function.
dnl
dnl If a module is found, then the module-name will be returned via 
dnl this variable.
dnl
AC_DEFUN([AC_MY_CHECK_MODULE],[
  if test x${found_$1} = xno; then
    AC_ARG_ENABLE([$2],[AC_HELP_STRING([--disable-$2],[disable check for $2])],[
      if test x$enableval = xno; then
        AC_MSG_CHECKING([for $2])
        AC_MSG_RESULT([check disabled])
      else
        if test "x$3" = "x"; then
          AC_MSG_CHECKING([for $2 while ])
          PKG_CHECK_MODULES([$1],[$2],[found_$1="$2"],[found_$1=no])
        else
          AC_MSG_CHECKING([for $2 $3 $4 while ])
          PKG_CHECK_MODULES([$1],[$2 $3 $4],[found_$1="$2"],[found_$1=no])
        fi
      fi
    ],[
      if test "x$3" = "x"; then
        AC_MSG_CHECKING([for $2 while ])
        PKG_CHECK_MODULES([$1],[$2],[found_$1="$2"],[found_$1=no])
      else
        AC_MSG_CHECKING([for $2 $3 $4 while ])
        PKG_CHECK_MODULES([$1],[$2 $3 $4],[found_$1="$2"],[found_$1=no])
      fi
    ])
  fi
])
