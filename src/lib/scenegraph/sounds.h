/*
=INSERT_TEMPLATE_HERE=

$Id: sounds.h,v 1.2 2008/11/27 00:27:18 couannette Exp $

Sound engine client code

*/

#ifndef __FREEX3D_SOUND_CLIENT_H__
#define __FREEX3D_SOUND_CLIENT_H__


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


#endif /* __FREEX3D_SOUND_CLIENT_H__ */
