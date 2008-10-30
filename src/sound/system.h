/*********************************************************************
 *
 * FreeWRL SoundServer engine
 *
 *  Copyright (C) 2002 John Stewart, CRC Canada.
 *  DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 *  See the GNU Library General Public License (file COPYING in the distribution)
 *  for conditions of use and redistribution.
 *
 *
 *
 * Wav decoding data came from data sheets at:
 * http:www.borg.com/~jglatt/tech/wave.htm
 *
 * Some programming info from:
 * http: vengeance.et.tudelft.nl/ecfh/Articles/devdsp-0.1.txt
 *
 *
 *********************************************************************/

#ifndef FREEWRL_SOUND_SYSTEM_H
#define FREEWRL_SOUND_SYSTEM_H

#if STDC_HEADERS
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#else
# if !HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# if !HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_SYS_IPC_H
# include <sys/ipc.h>
#endif

#include <linux/soundcard.h>


#endif /* FREEWRL_SOUND_SYSTEM_H */
