/*
=INSERT_TEMPLATE_HERE=

$Id: EAIServ.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

Implement EAI server functionality for FreeWRL.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "../input/EAIheaders.h"


/*									*/
/* Design notes:							*/
/*	FreeWRL is a server, the Java (or whatever) program is a client	*/
/*									*/
/*	Commands come in, and get answered to, except for sendEvents;	*/
/*	for these there is no response (makes system faster)		*/
/*									*/
/*	Nodes that are registered for listening to, send async		*/
/*	messages.							*/
/*									*/
/*	very simple example:						*/
/*		move a transform; Java code:				*/
/*									*/
/*		EventInMFNode addChildren;				*/
/*		EventInSFVec3f newpos;					*/
/*		try { root = browser.getNode("ROOT"); }			*/
/*		catch (InvalidNodeException e) { ... }			*/
/*									*/
/*		newpos=(EventInSFVec3f)root.getEventIn("translation");	*/
/*		val[0] = 1.0; val[1] = 1.0; val[2] = 1.0;		*/
/*		newpos.setValue(val);					*/
/*									*/
/*		Three EAI commands sent:				*/
/*			1) GetNode ROOT					*/
/*				returns a node identifier		*/
/*			2) GetType (nodeID) translation			*/
/*				returns posn in memory, length,		*/
/*				and data type				*/
/*									*/
/*			3) SendEvent posn-in-memory, len, data		*/
/*				returns nothing - assumed to work.	*/
/*									*/
/************************************************************************/


int EAIport = 9877;				/* port we are connecting to*/
int EAIinitialized = FALSE;		/* are we running?*/
int EAIfailed = FALSE;			/* did we not succeed in opening interface?*/
char EAIListenerData[EAIREADSIZE]; /* this is the location for getting Listenered data back again.*/
char EAIListenerArea[40];

/* socket stuff */
int 	EAIsockfd = -1;			/* main TCP socket fd*/
int	EAIlistenfd = -1;			/* listen to this one for an incoming connection*/
fd_set rfds2;
struct timeval tv2;


struct sockaddr_in	servaddr, cliaddr;
unsigned char loopFlags = 0;

enum theLoopFlags {
		NO_CLIENT_CONNECTED = 0x1,
		NO_EAI_CLASS	    = 0x2,
		NO_RETVAL_CHANGE    = 0x4
};

/* EAI input buffer */
char *EAIbuffer;
int EAIbufcount;				/* pointer into buffer*/
int EAIbufsize;				/* current size in bytes of input buffer*/

int EAIwanted = FALSE;                       /* do we want EAI?*/

/* open the socket connection -  we open as a TCP server, and will find a free socket */
/* EAI will have a socket increment of 0; Java Class invocations will have 1 +	      */
int conEAIorCLASS(int socketincrement, int *EAIsockfd, int *EAIlistenfd) {
	int len;
	const int on=1;
	int flags;

        struct sockaddr_in      servaddr;

	if ((EAIfailed) &&(socketincrement==0)) return FALSE;

	if ((*EAIsockfd) < 0) {
		/* step 1  - create socket*/
	        if (((*EAIsockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf ("EAIServer: socket error\n");
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}

		setsockopt ((*EAIsockfd), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		if ((flags=fcntl((*EAIsockfd),F_GETFL,0)) < 0) {
			printf ("EAIServer: trouble gettingsocket flags\n");
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		} else {
			flags |= O_NONBLOCK;

			if (fcntl((*EAIsockfd), F_SETFL, flags) < 0) {
				printf ("EAIServer: trouble setting non-blocking socket\n");
				loopFlags &= ~NO_EAI_CLASS;
				return FALSE;
			}
		}

		if (eaiverbose) { 
		printf ("conEAIorCLASS - socket made\n");
		}


		/* step 2 - bind to socket*/
	        bzero(&servaddr, sizeof(servaddr));
	        servaddr.sin_family      = AF_INET;
	        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	        servaddr.sin_port        = htons(EAIport+socketincrement);
		/*printf ("binding to socket %d\n",EAIport+socketincrement);*/

	        while (bind((*EAIsockfd), (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}

		if (eaiverbose) { 
		printf ("EAISERVER: bound to socket %d\n",EAIBASESOCKET+socketincrement);
		}


		/* step 3 - listen*/

	        if (listen((*EAIsockfd), 1024) < 0) {
	                printf ("EAIServer: listen error\n");
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}
	}

	if (((*EAIsockfd) >=0) && ((*EAIlistenfd)<0)) {
		/* step 4 - accept*/
		len = sizeof(cliaddr);
	        if ( ((*EAIlistenfd) = accept((*EAIsockfd), (struct sockaddr *) &cliaddr, (socklen_t *)&len)) < 0) {
			if (eaiverbose) {
			if (!(loopFlags&NO_CLIENT_CONNECTED)) {
				printf ("EAISERVER: no client yet\n");
				loopFlags |= NO_CLIENT_CONNECTED;
			}
			}

		} else {
			loopFlags &= ~NO_CLIENT_CONNECTED;
			if (eaiverbose) {
				printf ("EAISERVER: no client yet\n");
			}
		}
	}


	/* are we ok, ? */
	if ((*EAIlistenfd) >=0)  {
		/* allocate memory for input buffer */
		EAIbufcount = 0;
		EAIbufsize = 2 * EAIREADSIZE; /* initial size*/
		EAIbuffer = (char *)MALLOC(EAIbufsize * sizeof (char));

		/* zero out the EAIListenerData here, and after every use */
		bzero(&EAIListenerData, sizeof(EAIListenerData));

		/* seems like we are up and running now, and waiting for a command */
		/* and are we using this with EAI? */
		if (socketincrement==0) EAIinitialized = TRUE;
	}
	/* printf ("EAISERVER: conEAIorCLASS returning TRUE\n");*/

	if (eaiverbose) {
	if ( !(loopFlags&NO_EAI_CLASS)) {
		printf ("EAISERVER: conEAIorCLASS returning TRUE\n");
		loopFlags |= NO_EAI_CLASS;
	}
	}

	return TRUE;
}


/* the user has pressed the "q" key */
void shutdown_EAI() {

	if (eaiverbose) { 
	printf ("shutting down EAI\n");
	}

	strcpy (EAIListenerData,"QUIT\n\n\n");
	if (EAIinitialized) {
		EAI_send_string(EAIListenerData,EAIlistenfd);
	}

}

void create_EAI()
{
        if (eaiverbose) { 
	printf ("EAISERVER:create_EAI called\n");
	}


	/* already wanted? if so, just return */
	if (EAIwanted) return;

	/* so we know we want EAI */
	EAIwanted = TRUE;

	/* have we already started? */
	if (!EAIinitialized) {
		EAIfailed = !(conEAIorCLASS(0,&EAIsockfd,&EAIlistenfd));
	}
}


/* possibly we have an incoming EAI request from the client */
void handle_EAI () {
	/* do nothing unless we are wanted */
	if (!EAIwanted) return;
	if (!EAIinitialized) {
		EAIfailed = !(conEAIorCLASS(0,&EAIsockfd,&EAIlistenfd));
		return;
	}

	/* have we closed connection? */
	if (EAIlistenfd < 0) return;

	EAIbufcount = 0;

	EAIbuffer = read_EAI_socket(EAIbuffer,&EAIbufcount, &EAIbufsize, &EAIlistenfd);
	/* printf ("read, EAIbufcount %d EAIbufsize %d\n",EAIbufcount, EAIbufsize); */

	/* make this into a C string */
	EAIbuffer[EAIbufcount] = 0;
	if (eaiverbose) {
		if (EAIbufcount) printf ("handle_EAI-- Data is :%s:\n",EAIbuffer);
	}

	/* any command read in? */
	if (EAIbufcount > 1)
		EAI_parse_commands ();
}


void EAI_send_string(char *str, int lfd){
	unsigned int n;

	/* add a trailing newline */
	strcat (str,"\n");

	if (eaiverbose) {
		printf ("EAI/CLASS Command returns\n%s(end of command)\n",str);
	}

	/* printf ("EAI_send_string, sending :%s:\n",str); */
	n = write (lfd, str, (unsigned int) strlen(str));
	if (n<strlen(str)) {
		if (eaiverbose) {
		printf ("write, expected to write %d, actually wrote %d\n",n,strlen(str));
		}
	}
	/* printf ("EAI_send_string, wrote %d\n",n); */
}


/* read in from the socket.   pass in -
	pointer to buffer,
	pointer to buffer index
	pointer to max size,
	pointer to socket to listen to

 	return the char pointer - it may have been REALLOC'd */


char *read_EAI_socket(char *bf, int *bfct, int *bfsz, int *EAIlistenfd) {
	int retval, oldRetval;

	/* if (eaiverbose) printf ("read_EAI_socket, thread %d EAIlistenfd %d buffer addr %d time %lf\n",pthread_self(),*EAIlistenfd,bf,TickTime); */
	retval = FALSE;
	do {
		tv2.tv_sec = 0;
		tv2.tv_usec = 0;
		FD_ZERO(&rfds2);
		FD_SET((*EAIlistenfd), &rfds2);

		oldRetval = retval;
		retval = select((*EAIlistenfd)+1, &rfds2, NULL, NULL, &tv2);
		/* if (eaiverbose) printf ("select retval %d\n",retval); */

		if (retval != oldRetval) {
			loopFlags &= NO_RETVAL_CHANGE;
		}

		if (eaiverbose) {
		if (!(loopFlags&NO_RETVAL_CHANGE)) {
			printf ("readEAIsocket--, retval %d\n",retval);
			loopFlags |= NO_RETVAL_CHANGE;
		}
		}


		if (retval) {
			retval = read ((*EAIlistenfd), &bf[(*bfct)],EAIREADSIZE);
			if (retval <= 0) {
				if (eaiverbose) {
					printf ("read_EAI_socket, client is gone!\n");
				}

				/*perror("READ_EAISOCKET");*/
				/* client disappeared*/
				close ((*EAIlistenfd));
				(*EAIlistenfd) = -1;

				/* And, lets just exit FreeWRL*/
				printf ("FreeWRL:EAI socket closed, exiting...\n");
				doQuit();
			}

			if (eaiverbose) {
			    char tmpBuff1[EAIREADSIZE];
			    strncpy(tmpBuff1,&bf[(*bfct)],retval);
			    tmpBuff1[retval] = '\0';
			    printf ("read in from socket %d bytes, max %d bfct %d cmd <%s>\n",
				    retval,EAIREADSIZE, *bfct,tmpBuff1);/*, &bf[(*bfct)]);*/
			}


			(*bfct) += retval;

			if (((*bfsz) - (*bfct)) <= EAIREADSIZE) {
				if (eaiverbose) 
				printf ("read_EAI_socket: HAVE TO REALLOC INPUT MEMORY:bf %x bfsz %d bfct %d\n",bf,*bfsz, *bfct);  
				(*bfsz) += EAIREADSIZE;
				/* printf ("read_EAI_socket: bfsz now %d\n",*bfsz); */
				bf = (char *)REALLOC (bf, (unsigned int) (*bfsz));
				/* printf ("read_EAI_socket: REALLOC complete\n"); */
			}
		}
	} while (retval);
	return (bf);
}

