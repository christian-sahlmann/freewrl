/*
=INSERT_TEMPLATE_HERE=

$Id: SoundEngineClient.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

This is the SoundEngine client code for FreeWRL.

Some of this stuff came from files from "wavplay"  - see information below

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

/* #include "../vrml_parser/Structs.h" */
/* #include "headers.h" */
/* #include "../vrml_parser/CParseGeneral.h" */
/* #include "../world_script/jsUtils.h" */
/* #include "../world_script/CScripts.h" */
/* #include "Snapshot.h" */
/* #include "../scenegraph/Collision.h" */
/* #include "../scenegraph/quaternion.h" */
/* #include "../scenegraph/Viewer.h" */
/* #include "../input/SensInterps.h" */
/* #include "../x3d_parser/Bindable.h" */
/* #include "../input/EAIheaders.h" */
#include "../scenegraph/sounds.h"


int SReg[MAXSOUNDS];

int my_ipc_key;

FWSNDMSG msg;		/* message buffer */

/* TODO: integrate this variable into configure */
char sspath[] = "freex3d_snd" /*SOUNDSERVERBINARY*/; /* compile line flag */

static int initialized = SOUND_NEEDS_STARTING; /* are we able to run? */


/* IPC stuff */
#ifndef __APPLE__
int msq_toserver = -1;
int msq_fromserver = -1;
#else
char* serverpipe = "/tmp/soundserver";
char* clientpipe = "/tmp/soundclient";
int server_pipe_fd, client_pipe_fd;
time_t last_time, current_time;
#endif

pid_t S_Server_PID;

void Sound_toserver (char *message) {
	int xx;

	if (initialized != SOUND_STARTED)  return;

	strcpy (msg.msg,message);
	/* printf ("Client:Sending to server %s\n",msg.msg); */
#ifndef __APPLE__
        while(xx = msgsnd(msq_toserver, &msg,strlen(msg.msg)+1,IPC_NOWAIT));
#else
	xx = write(server_pipe_fd, &msg, sizeof(msg));
	if (xx > 0)
		xx = 0;
#endif
	/* printf ("Client:Sent to server\n"); */
        if (xx) {   /* Send to server */
		perror("server error");
                printf ("SoundEngineServer - error sending ready msg\n");
#ifndef __APPLE__
                initialized = SOUND_FAILED;
#endif
        }
}




void SoundEngineInit () {
	int x;
	char buf[200];

	struct stat enginestat;

	/* have we done this before (can happen if more than 1 sound source) */
	if (initialized != SOUND_NEEDS_STARTING) return;

	/* is the sound engine installed on this system? */
	if (stat(sspath,&enginestat)) {
		printf ("FreeWRL: SoundEngine not installed on system\n");
		initialized = SOUND_FAILED;
		return;
	}

	my_ipc_key = getpid();
	/* my_ipc_key = 1234; */

	msg.mtype=1;

	/* initialize SoundRegistered "database" */
	for (x=0; x<MAXSOUNDS; x++) SReg[x]=FALSE;

	/* printf ("Client, thus queue key is %d\n",my_ipc_key); */

	/* message queue for client/server comms */
#ifndef __APPLE__
	if ( (msq_toserver = msgget(my_ipc_key,IPC_CREAT|0666)) < 0 ) {
		ConsoleMessage ("FreeWRL:SoundServer error creating toserver message queue\n");
		initialized = SOUND_FAILED;
		return;
	}
	if ( (msq_fromserver = msgget(my_ipc_key+1,IPC_CREAT|0666)) < 0 ) {
		ConsoleMessage ("FreeWRL:SoundServer error creating fromserver message queue\n");
		initialized = SOUND_FAILED;
		return;
	}
#else

	if ((client_pipe_fd = open (clientpipe, O_RDONLY | O_NONBLOCK)) < 0) {
		if ((mkfifo(clientpipe, S_IRUSR | S_IWUSR | S_IXUSR)) < 0) {
			ConsoleMessage ("FreeWRL:SoundServer error creating client pipe\n");
			initialized = SOUND_FAILED;
			return;
		}
		if ((client_pipe_fd = open (clientpipe, O_RDONLY | O_NONBLOCK)) < 0) {
			ConsoleMessage ("FreeWRL:SoundServer error opening client pipe\n");
			initialized = SOUND_FAILED;
			return;
		}
	}
#endif
	/* printf ("Client - msq_toserver=%x, msq_fromserver=%x.\n", msq_toserver,msq_fromserver); */

	sprintf(buf,"INIT %d",my_ipc_key);
/* printf("buf='%s' sspath='%s'.\n",buf,sspath); */

	if ( (S_Server_PID = fork()) == (pid_t)0L ) {
		/* is this path ok? */
		execl((const char *)sspath,(const char *)buf,"",NULL);

		/* if we got here, we have an error... */
		printf("FreeWRL:SoundServer:%s: exec of %s\n",
			strerror,sspath);
#ifndef __APPLE__
		msgctl(msq_toserver,IPC_RMID,NULL);
		msgctl(msq_fromserver,IPC_RMID,NULL);
#else
	fclose((FILE*)client_pipe_fd);
#endif
		initialized = SOUND_FAILED;
		return;

	} else if ( S_Server_PID < 0 ) {
		ConsoleMessage ("FreeWRL:SoundServer %s: error starting server process",
			strerror);
#ifndef __APPLE__
		msgctl(msq_toserver,IPC_RMID,NULL);
		msgctl(msq_fromserver,IPC_RMID,NULL);
#else
		fclose((FILE*)client_pipe_fd);
#endif
		initialized = SOUND_FAILED;
		return;
	}


	/* printf ("Client: - server pid %d\n",S_Server_PID); */

	/* if FreeWRL actually gets to the exit stage... :-) */
	atexit(SoundEngineDestroy);

	/* wait for the message queue to initialize. */
	waitformessage();

#ifdef __APPLE__
	if ((server_pipe_fd = open (serverpipe, O_WRONLY | O_NONBLOCK)) < 0) {
		perror("Open error\n");
		printf("FreeWRL:SoundServer error opening server pipe\n");
		initialized = SOUND_FAILED;
		return;
	}
#endif
	if (initialized == SOUND_FAILED) {
		printf("FreeWRL:SoundServer: Timeout: starting server.");
		SoundEngineDestroy();
	}
}

/* Wait for SoundServer to return a response. Note: Not all commands wait for this return. */
void waitformessage () {
	int xx;
	time_t t0, t;
	pid_t PID;
	int proc_status;

	time(&t0);

	while ( 1 ) {

		/* wait for a response - is the server telling us it is ok? */
		/* printf ("Client: waiting for response on %d\n",msq_toserver); */
		/* printf("Client: waiting for response\n"); */

		do {
#ifndef __APPLE__
			xx = msgrcv(msq_fromserver,&msg,128,1,0);
#else
	 		xx = read (client_pipe_fd, &msg, sizeof(msg));
			if (xx <= 1)
				xx = 0;
#endif
			/* printf ("Client waiting... xx is %d\n",xx); */

			usleep(1000);
		} while (!xx);

		/* printf ("message received was %s\n", msg.msg);  */
		if (xx>0) {
			 /* We have a message from the server */
			if ( msg.mtype == 1 ) {
				initialized = SOUND_STARTED;
				return;	/* connect OK */
			}
		} else	{
			while ((PID=waitpid(-1,&proc_status,WNOHANG)) == -1
				&& errno==EINTR );
			if ( PID > 0 ) {
				ConsoleMessage ("FreeWRL:SoundServer process ID %ld terminated: %d",
					PID,proc_status);
				initialized = SOUND_FAILED;
				return;

			} else	sleep(1);
		}

		time(&t);
		if ( t - t0 > 5 )
			break;
	}

}

/* close socket, destroy the server */
void SoundEngineDestroy() {
	/* printf("reached DESTROY\n"); */
	if (initialized == SOUND_STARTED) {
#ifndef __APPLE__
		msgctl(msq_toserver,IPC_RMID,NULL);
		msgctl(msq_fromserver,IPC_RMID,NULL);
#else
	/* fclose((FILE*)serverpipe); */
	/* fclose((FILE*)clientpipe); */
	/* unlink(serverpipe); */
	/* unlink(clientpipe); */
#endif
		printf ("SoundEngineDestroy, sound was started successfully\n");
		kill(S_Server_PID,SIGTERM);
	}
	initialized = SOUND_NEEDS_STARTING;
}

int SoundSourceRegistered  (int num) {
	if (num >= MAXSOUNDS) {
		printf ("Too many sounds in VRML file - max %d",num);
		return FALSE;
	}
	return SReg[num];
}

float SoundSourceInit (int num, int loop, double pitch, double start_time, double stop_time,
		char *url) {

	char mystring[512];
	float duration;
	int returnednum;


	SReg[num] = TRUE;
	/* printf ("start of SoundSourceInit)\n");
		printf ("num %d\n",num);
		printf ("loop %d\n",loop);
		printf ("pitch %f\n",pitch);
		printf ("start_time %f\n",start_time);
		printf ("stop_time %f\n",stop_time);
		printf ("SoundSourceInit - url is %s\n",url);
	*/
	if (url == NULL) {
		printf ("SoundSourceInit - no file to source \n");
		return 0.0;
	}

	if (strlen(url) > 192) {
		printf ("SoundSourceInit - url %s is too long\n",url);
		return 0.0;
	}

	sprintf (mystring,"REGS:%s %2d %2d %4.3f %4.3f %4.3f",url,num,loop,pitch,start_time,
			stop_time);
	Sound_toserver(mystring);
	/* printf ("SoundSourceInit, waiting for response\n"); */
	waitformessage();
	/* printf ("SoundSourceInit, got message %s\n",msg.msg); */
	if (sscanf (msg.msg,"REGS %d %f",&returnednum,&duration) != 2) {
		/* something funny happened here */
		return 1.0;
	} else {
		return duration;
	}
}

/* send new active state to the soundengine. */
void SetAudioActive (int num, int stat) {
	char mystring[512];

	/* printf ("SoundSource - got SetAudioActive for %d state %d\n",num,stat); */
	sprintf (mystring,"ACTV %2d %2d",num,stat);
	Sound_toserver(mystring);
}
