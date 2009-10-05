
/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

/*********************************************************************
 *
 * FreeWRL SoundServer engine
 *
 *  Copyright (C) 2002 John Stewart, CRC Canada.
 *  DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 *  See the GNU Library General Public License (file COPYING in the distribution)
 *  for conditions of use and redistribution.
 *
 * Wav decoding data came from data sheets at:
 * http:www.borg.com/~jglatt/tech/wave.htm
 *
 * Some programming info from:
 * http: vengeance.et.tudelft.nl/ecfh/Articles/devdsp-0.1.txt
 *
 *********************************************************************/

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "system.h"
#include "soundheader.h"
#include "../input/InputFunctions.h"


int freewrlSystem (char *string);

key_t IPCKey;
int msq_fromclnt;
int msq_toclnt;

int current_max = -1;		/*  the maximum source number recieved so far.*/
int registered[MAXSOURCES];	/*  is source registered? (boolean)*/
int active[MAXSOURCES];		/*  is source active? (boolean)*/
int loop[MAXSOURCES];		/*  is this sound looped? (boolean)*/
SNDFILE *sndfile[MAXSOURCES];	/*  structure containing sound file info*/

FWSNDMSG msg;	/*  incoming message*/

float fps = 30.0;		/*  initial value...*/
int currentSource = -1;

char cp2[310];    /*  hold the current filename, in case of errors*/

int S_Server_IPC = -1;
int xx;

unsigned char* data;


/*  
 * Sound file handling routines
 */

/*  if the sndfile is open, rewind and set bytes_remaining*/
void rewind_to_beginning (SNDFILE *wavfile) {
	if (wavfile != NULL) {
		if (wavfile->fd != NULL) {
			/* printf ("rewinding to beginning...\n");*/
			/* printf ("seek set is %d, chunkSize %d\n",*/
			/* 		(int)wavfile->wavdataoffset,*/
			/* 		(int)wavfile->DataChunk.chunkSize);*/

			wavfile->bytes_remaining = wavfile->DataChunk.chunkSize;
			fseek (wavfile->fd, wavfile->wavdataoffset, SEEK_SET);
			/* printf ("rewind bytes remaining %ld\n",wavfile->bytes_remaining);*/

			if (wavfile->bytes_remaining <= 0) {
				printf ("Error in getting wavfile DataChunk\n");
				wavfile->fd = NULL;
				return;
			}
		}
	}
}


/*  find a chunk start.*/
int chunk (char *buf, char *find, int len) {
	int mycnt;

	mycnt=0;
	while (mycnt < (len) - strlen(find)) {
		if (strncmp (&buf[mycnt], find, strlen(find)) == 0) {
			/*  found it!*/
			/* printf ("found %s at %d\n",find, mycnt);*/
			return (mycnt);
		} else {
			/* printf ("not found, mycnt = %d\n",mycnt);*/
			mycnt ++;
		}
	}
	return -1;  /*  did not find it*/
}


/*  Decode what kind of file this is - skip through the headers,*/
/*  save the type in the SNDFILE structure.*/

int querySoundType(SNDFILE *me) {
	int br;

	br = fread(me->data,1,BUFSIZE,me->fd);
	me->dataptr = chunk (me->data,"RIFF",BUFSIZE);

	/* Not a RIFF file*/
	if (me->dataptr < 0) {
		printf ("SoundEngine:not a RIFF file\n\t%s\n",cp2);
		return -1;
	}

	br = chunk (&me->data[me->dataptr],"WAVE",BUFSIZE);
	/*  a WAVE file*/
	if (br < 0) {
		printf ("SoundEngine:not a WAVE file\n\t%s\n",cp2);
		return -1;
	}

	me->dataptr +=br;
	br = chunk (&me->data[me->dataptr],"fmt ",BUFSIZE);
	/*  have format*/
	if (br < 0) {
		printf ("SoundServer:no fmt found in WAVE file\n\t%s\n",cp2);
		return -1;
	}

	me->dataptr += br;

	/*  copy over format header information*/
	memcpy (&me->FormatChunk, &me->data[me->dataptr], sizeof (fmtChnk));

	/*
	  printf ("fmt chunkid %c%c%c%c\n",me->FormatChunk.chunkID[0],
	  me->FormatChunk.chunkID[1],me->FormatChunk.chunkID[2],me->FormatChunk.chunkID[3]);
	  printf ("fmt chunkSize %ld\n", me->FormatChunk.chunkSize);
	  printf ("fmt wChannels %d\n", me->FormatChunk. wChannels);
	  printf ("fmt wFormatTag %d\n", me->FormatChunk. wFormatTag);
	  printf ("fmt dwSamplesPerSec %ld\n", me->FormatChunk. dwSamplesPerSec);
	  printf ("fmt dwAvgBytesPerSec %ld\n", me->FormatChunk. dwAvgBytesPerSec);
	  printf ("fmt wBlockAlign %d\n", me->FormatChunk. wBlockAlign);
	  printf ("fmt wBitsPerSample %d\n", me->FormatChunk. wBitsPerSample);
	*/

	if (me->FormatChunk. wFormatTag != 1) {
		printf ("SoundServer:compressed WAV not handled yet\n\t%s\n",
				cp2);
		return -1;
	}

	/*  pass over the fmt chunk - note the chunkSize does not include all. see spec.*/
	me->dataptr += 8 + me->FormatChunk.chunkSize;


	br = chunk (&me->data[me->dataptr],"data",BUFSIZE);
	/*  have data*/
	if (br < 0) {
		printf ("SoundServer:no data found in WAVE file\n\t%s\n",cp2);
		return -1;
	}

	me->dataptr += br;
	memcpy (&me->DataChunk, &me->data[me->dataptr], sizeof (datChnk));

	/*
	  printf ("data chunkid %c%c%c%c\n",me->DataChunk.chunkID[0],
	  me->DataChunk.chunkID[1],me->DataChunk.chunkID[2],me->DataChunk.chunkID[3]);
	  printf ("data chunkSize %lx\n", me->DataChunk.chunkSize);
	  printf ("actual number of sample frames %ld\n",me->DataChunk.chunkSize/me->FormatChunk.wBlockAlign);
	  printf ("dataptr is %d\n",me->dataptr);
	*/

	/*  does this file have a zero chunksize?*/
	if (me->DataChunk.chunkSize <= 0) {
		printf ("SoundServer:WAV DataChunk size invalid\n\t%s\n",cp2);
		return -1;
	}

	/*  is this file compressed?*/

	me->wavdataoffset = me->dataptr+8; /*  wavdataoffset is the actual position of start of data*/
	return WAVFILE;
}

/*  Open and initiate sound file*/

SNDFILE *openSound (char *path,int soundNo) {

	SNDFILE *mysound;

	mysound	= (SNDFILE *) malloc (sizeof(SNDFILE)); /*  This is the return value*/

	if (!mysound) return NULL;	/*  memory allocation error*/

	mysound->fd = fopen(path,"r");
	mysound->bytes_remaining = UNINITWAV;

	if (mysound->fd == NULL) {
		free (mysound);
		return NULL;
	}

	/*  ok - we have the file opened. Assume WAV file, because that's*/
	/*  what we handle for now.*/

	switch (querySoundType(mysound)) {
	case WAVFILE: {
		return initiateWAVSound(mysound,soundNo);
		break;
	}
	case MP3FILE: {
		mysound->type = MP3FILE;
		break;
	}
	case MPGFILE: {
		mysound->type = MPGFILE;
		break;
	}
	default: {
		printf ("unknown file type: %s\n",cp2);
		free (mysound);
		return NULL;
	}
	}
	/*  we should never reach here...*/
	return NULL;
}

/*
 * Receive information from FreeWRL
 */
void toclnt(char *message_to_send) {
	msg.mtype= 1;
	(void) strcpy(msg.msg, message_to_send);
	/* printf ("SoundEngine - sending back %s\n",msg.msg);*/

	while((xx=msgsnd(msq_toclnt, &msg,strlen(msg.msg)+1,IPC_NOWAIT)) != 0);
	if (xx) {   /* Send to client */
		printf ("SoundEngineServer - error sending ready msg\n");
		exit(1);
	}
	/* printf ("SoundEngine - sendT back %s\n",msg.msg);*/
}

int fromclnt () {
	return msgrcv(msq_fromclnt,&msg,256,1,0);
}


/*  Go through, and act on the message -it is stored in the global "msg" struct*/
void process_command () {
	float x,y,z; /*  temporary variables*/
	int a,b,cp2len;   /*  temporary variables*/
	int myloop;
	int mysource;
	float bal;	/*  balance*/
	char cp[310];    /*  temporary variable*/
	char st[10];
	char pitch[30];
	double duration;

	/* printf ("processing %s\n",msg.msg);*/

	if (strncmp ("REGS",msg.msg,4) == 0) {
		/*  a REGISTER message*/
		a=5; b=0;
		/* printf ("REGS matched len %d, first start %c %c %c %c\n",strlen(msg.msg),*/
		/* 		msg.msg[a],msg.msg[a+1], msg.msg[a+2], msg.msg[a+3]);*/

		/*  start SOX conversion...*/
		cp[0]='\0';
		strcpy(cp,SOUNDCONV);
		strcat(cp," ");
		b = strlen(cp);
		cp2len=0; /*  keep the original file name around for a bit.*/

		/*  copy over the url name; skip past the REGS: at beginning.*/
		while ((a<strlen(msg.msg)-1) && (b<300) && (msg.msg[a]>' ')) {
			cp[b]=msg.msg[a];
			cp2[cp2len] = msg.msg[a];
			b++; a++; cp2len++;
		}
		cp[b]='\0'; cp2[cp2len]='\0';

		/*  get rest of parameters*/
		/* printf ("getting rest of parameters from %s\n",&msg.msg[a]);*/
		sscanf (&msg.msg[a], " %d %d %f %f %f",&mysource,&myloop,&x,&y,&z);

		/*  do the pitch*/
		strcat (cp, " -r ");
		if ((x > 1.01) || (x < 0.98)) {
			if (x<0.01) x = 1;
			sprintf (pitch,"%d ",(int) ((float)22050.0/x));
		} else {
			sprintf (pitch,"%d ",22050);
		}
		strcat (cp,pitch);

		/*  finish the conversion line*/
		strcat (cp,"-c2 -w /tmp/sound");
		b = strlen(cp);

		sprintf (st,"%d.wav",mysource);
		/* printf ("ST is %s\n cp is %s\n",st,cp);*/
		strcat (cp,st);

		/* strcat (cp, " 2>/tmp/FreeWRL_Errors");*/

		/* printf ("going to system %s\n",cp); */
		freewrlSystem (cp);

		/*  make the new, converted file name, then later, open it*/
		strcpy (cp,"/tmp/sound");
		strcat (cp,st);

		/* printf ("registering source %d loop %d x %f y %f z %f name %s \n",mysource,myloop,x,y,z,cp);*/

		if (mysource > current_max) current_max = mysource;


		/*  Can we open this sound file?*/

		/* printf ("REGS opening sound\n");*/
		sndfile[mysource] = openSound(cp,mysource);
		if (sndfile[mysource] == NULL) {
			printf ("SoundServer:open problem for:\n\t %s\n",cp2);
			duration = 1.0;
		} else {
			/*  Copy over all of the temporary data to the correct place.*/
			registered[mysource] = 1;     /*  is source registered? (boolean)*/
			loop[mysource] = myloop;           /*  is this sound looped? (boolean)*/
			sndfile[mysource]->pitch = x;          /*  pitch of 1 = standard playback*/

			sndfile[mysource]->ampl = 0;         /*  Gain of this sound*/
			sndfile[mysource]->balance = 50;	/*  balance of this sound.*/

			duration = (double) sndfile[mysource]->DataChunk.chunkSize / (double) sndfile[mysource]->FormatChunk.dwAvgBytesPerSec;

		}
		sprintf (cp, "REGS %d %f",mysource,(float)duration);
		toclnt(cp);			/* Tell client we're ready */

	} else if (strncmp ("AMPL",msg.msg,4) == 0) {
		/*  set amplitude for this sound source*/

/* 	printf ("%s\n",msg.msg);*/
		/* format is command, source#, amplitude, balance, Framerate */

		sscanf (msg.msg,"AMPL %d %f %f %f",&a,&x,&bal,&fps);
		/* printf ("got ampl for sound %d\n",a);*/
		if ((registered[a] == 1) && (a>=0) && (a<MAXSOURCES)) {
			sndfile[a]->ampl = (int) (x*100.0);
			/* printf ("ampl conv, orig %f now %d\n",x,sndfile[a]->ampl);*/
			sndfile[a]->balance = (int) ((float)bal * 100.0);
		}
		playWavFragment ();
	} else if (strncmp ("ACTV",msg.msg,4) == 0) {
		/*  set this source to be active*/
		sscanf (msg.msg,"ACTV %d %d",&a,&b);
		if ((a>=0) && (a<MAXSOURCES)) {
			active[a]=b;
			if (b==1) {
				/*  sound is becoming active*/
				rewind_to_beginning (sndfile[a]);
			}
		}
		/* printf ("ACTV parsing, active%d now is %d from message %s\n",a,b,msg.msg);*/

		/* } else {*/
		/* 	printf ("SoundEngine - unknown message recieved %s\n",msg.msg);*/
	}
}


int main(int argc,char **argv) {
	int count;
	char fileRemove[200];

	/* FIXME: argc is minimum 1 since argv[0] contains program's name */
	if (argc <1) {
		printf ("Server: too few args\n");
		exit(1);
	}

	if ((argc == 2) && !strcmp(argv[1],"-v")) {
		printf("FreeWRL sound server\nVersion: %s\n", freewrl_snd_get_version());
		exit(0);
	}

	/*  initiate tables*/
	for (xx=0; xx<MAXSOURCES; xx++) {
		registered[xx] = 0;
		active[xx] = 0;
		sndfile[xx] = NULL;
	}

	/*  open the DSP*/
	initiateDSP();

	/* printf ("Server - getting the client IPC from argv %s\n", argv[0]);*/
	S_Server_IPC=getppid();

	/* printf ("a='%s', msg='%s', d='%d'.\n", argv[0],msg.msg,S_Server_IPC);*/
	if (!strncmp("INIT",argv[0],4)) {
		sscanf (argv[0],"%s%d",msg.msg,&S_Server_IPC);
	} else {
		printf ("SoundServer: no Client_IPC on command line\n");
		/* printf ("a='%s', msg='%s', dud='%d'.\n", argv[0],msg.msg,dud);*/
		exit(1);
	}

	/*  get message queues*/
	if ((msq_fromclnt = msgget(S_Server_IPC,0666)) < 0) {
		printf ("SoundServer: no IPC queue available\n");
		exit(1);
	}
	if ((msq_toclnt = msgget(S_Server_IPC+1,0666)) < 0) {
		printf ("SoundServer: no IPC queue available\n");
		exit(1);
	}
	/* printf ("Server, ok, msq_fromclnt=%x msq_toclnt=%x key %d\n",*/
	/* 		msq_fromclnt,msq_toclnt,S_Server_IPC);*/


	toclnt("OK");			/* Tell client we're ready */

	do {
		xx = fromclnt();
		if (xx < 0) {
			/*  gets here if the client exited*/
			exit (0);
		}

		/* printf ("server, from FreeWRL=%x message='%s'\n",xx,msg.msg);*/
		process_command ();
	} while (strncmp ("QUIT",msg.msg,4));
	for (count=0; count<current_max; count++) {
		sprintf (fileRemove,"/tmp/sound%d.wav",count);
		/* printf ("unlinking %d\n",count);*/
		unlinkShadowFile(fileRemove);
	}

	/* printf ("Server exiting normally\n");*/
	exit(0);
}

/* get all system commands, and pass them through here. What we do
 * is take parameters and execl them, in specific formats, to stop
 * people (or, to try to stop) from typing malicious code. */
int freewrlSystem (char *sysline) {

#define MAXEXECPARAMS 10
#define EXECBUFSIZE	2000
	int ok;
	char *paramline[MAXEXECPARAMS];
	char buf[EXECBUFSIZE];
	char *internbuf;
	int count;
	pid_t childProcess;
	int pidStatus;

	int waitForChild;

	waitForChild = TRUE;

	ok = FALSE;
	internbuf = buf;

	/* bounds check */
	if (strlen(sysline)>=EXECBUFSIZE) return FALSE;
	strcpy (buf,sysline);

	/* printf ("freewrlSystem, have %s here\n",internbuf);*/
	for (count=0; count<MAXEXECPARAMS; count++) paramline[count] = NULL;

	/* split the command off of internbuf, for execing. */
	count = 0;
	while (internbuf != NULL) {
		paramline[count] = internbuf;
		internbuf = strchr(internbuf,' ');
		if (internbuf != NULL) {
			/* printf ("more strings here! :%s:\n",internbuf);*/
			*internbuf = '\0';
			/* printf ("param %d is :%s:\n",count,paramline[count]);*/
			internbuf++;
			count ++;
			if (count >= MAXEXECPARAMS) return -1; /*  never...*/
		}
	}

/* 	 printf ("finished while loop, count %d\n",count);*/
/* 	{ int xx;*/
/* 		for (xx=0; xx<MAXEXECPARAMS;xx++) {*/
/* 			printf ("item %d is :%s:\n",xx,paramline[xx]);*/
/* 	}}*/


	/* is the last string "&"? if so, we don't need to wait around */
	if (strncmp(paramline[count],"&",strlen(paramline[count])) == 0) {
		waitForChild=FALSE;
		paramline[count] = '\0'; /*  remove the ampersand.*/
	}

	if (count > 0) {
		switch (childProcess=fork()) {
		case -1:
			perror ("fork"); exit(1);

		case 0: {
			int Xrv;

			/* child process */
			/* printf ("child execing, pid %d %d\n",childProcess, getpid());*/
		 	Xrv = execl(paramline[0],
						paramline[0],paramline[1], paramline[2],
						paramline[3],paramline[4],paramline[5],
						paramline[6],paramline[7],NULL);
			/* printf ("child finished execing\n");*/
			exit (Xrv);
		}
		default: {
			/* parent process */
			/* printf ("parent waiting for child %d\n",childProcess);*/

			/* do we have to wait around? */
			if (!waitForChild) {
				/* printf ("do not have to wait around\n");*/
				return 0;
			}
			waitpid (childProcess,&pidStatus,0);
			/* printf ("parent - child finished - pidStatus %d \n",*/
			/* 		pidStatus);*/
		}
		}
		return pidStatus;
	} else {
		printf ("System call failed :%s:\n",sysline);
	}
	return -1;
}

