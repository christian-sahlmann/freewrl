/*
=INSERT_TEMPLATE_HERE=

$Id: internal.c,v 1.1 2009/04/28 13:38:38 couannette Exp $

FreeWRL support library.
Internal functions: some very usefull functions are not always
present (example: strndup, ...).

*/


#include <config.h>
#include <system.h>
#include <internal.h>


#if !defined(HAVE_STRNDUP)

/******************************************************************************/
/* Jens Rieks sent in some changes - some of which uses strndup, which does not
   always exist... */
char *fw_strndup(const char *str, int len)
{
        char *retval;
        int ml;
        ml = strlen(str);
        if (ml > len) ml = len;
        retval = (char *) MALLOC (sizeof (char) * (ml+1));
        strncpy (retval,str,ml);
        /* ensure termination */
        retval[ml] = '\0';
        return retval;
}

#endif
