/*******************************************************************
 Copyright (C) 2003 John Stewart, CRC Canada.
 DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 See the GNU Library General Public License (file COPYING in the distribution)
 for conditions of use and redistribution.
*********************************************************************/

/* include file for sound engine client/server */


#ifndef __SOUNDS_H__
#define __SOUNDS_H__

#include "headers.h"

#include <math.h>

#ifdef AQUA
#include <gl.h>
#include <glu.h>
#include <glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


#ifndef __APPLE__
#include <sys/msg.h>

/* from Ian Stakenvicius */
#if defined(__linux__)
#include <unistd.h>
#endif
#endif
#if defined(__APPLE__)
#include <unistd.h>
#include <sys/uio.h>
#include <sys/ipc.h>
#endif


#define SNDMAXMSGSIZE 256

/* states of the sound engine */
#define SOUND_FAILED  2
#define SOUND_STARTED 1
#define SOUND_NEEDS_STARTING 3

#define MAXSOUNDS 50

typedef struct {
	long mtype;	/* message type */
	char	msg[SNDMAXMSGSIZE]; /* message data */
} FWSNDMSG;


void
Sound_toserver(char *message);

void
SoundEngineInit(void);

void
waitformessage(void);

void
SoundEngineDestroy(void);

int
SoundSourceRegistered(int num);

float
SoundSourceInit(int num,
				int loop,
				double pitch,
				double start_time,
				double stop_time,
				char *url);

void
SetAudioActive(int num, int stat);

#endif /* __SOUNDS_H__ */
