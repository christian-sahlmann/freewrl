/*
=INSERT_TEMPLATE_HERE=

$Id: EAIServ.c,v 1.27 2012/05/17 02:38:56 crc_canada Exp $

Implement EAI server functionality for FreeWRL.

*/


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



#include <config.h>

#if !defined(EXCLUDE_EAI)
#include <system.h>
#include <system_net.h>

/*JAS  - get this compiling on osx 10.4 ppc ==> system_net.h */

#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h" 
#include "../main/headers.h"

#include "EAIHeaders.h"

/************************************************************************/
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

unsigned char loopFlags = 0;

enum theLoopFlags {
		NO_CLIENT_CONNECTED = 0x1,
		NO_EAI_CLASS	    = 0x2,
		NO_RETVAL_CHANGE    = 0x4
};

typedef struct pEAIServ{
	pthread_mutex_t eaibufferlock;// = PTHREAD_MUTEX_INITIALIZER;

	int EAIport;// = 9877;				/* port we are connecting to*/
	int EAIinitialized;// = FALSE;		/* are we running?*/
	int EAIfailed;// = FALSE;			/* did we not succeed in opening interface?*/

	/* socket stuff */
	int 	EAIsockfd;// = -1;			/* main TCP socket fd*/
	fd_set rfds2;
	struct timeval tv2;

	struct sockaddr_in	servaddr, cliaddr;
	unsigned char loopFlags;// = 0;

	int EAIwanted;// = FALSE;                       /* do we want EAI?*/

#ifdef OLDCODE
OLDCODE	int EAIMIDIInitialized;// = FALSE; 	/* is MIDI eai running? */
OLDCODE	int EAIMIDIwanted;// = FALSE; 			/* do we want midi EAI? */
OLDCODE	int EAIMIDIfailed;// = FALSE;		/* did we not succeed in opening the MIDI interface? */
OLDCODE	int 	EAIMIDIsockfd;// = -1;		/* main TCP socket for MIDI EAI communication */
#endif //OLDCODE
}* ppEAIServ;



void *EAIServ_constructor()
{
	void *v = malloc(sizeof(struct pEAIServ));
	memset(v,0,sizeof(struct pEAIServ));
	return v;
}
void EAIServ_init(struct tEAIServ* t){
	//public
	t->EAIlistenfd = -1;			/* listen to this one for an incoming connection*/

#ifdef OLDCODE
OLDCODE	t->EAIMIDIlistenfd = -1;		/* listen on this socket for an incoming connection for MIDI EAI */
#endif //OLDCODE

	//private
	t->prv = EAIServ_constructor();
	{
		ppEAIServ p = (ppEAIServ)t->prv;
pthread_mutex_init(&(p->eaibufferlock), NULL);

p->EAIport = 9877;				/* port we are connecting to*/
p->EAIinitialized = FALSE;		/* are we running?*/
p->EAIfailed = FALSE;			/* did we not succeed in opening interface?*/

/* socket stuff */
p->EAIsockfd = -1;			/* main TCP socket fd*/
p->loopFlags = 0;

p->EAIwanted = FALSE;                       /* do we want EAI?*/
#ifdef OLDCODE
OLDCODEp->EAIMIDIsockfd = -1;		/* main TCP socket for MIDI EAI communication */
OLDCODEp->EAIMIDIwanted = FALSE; 			/* do we want midi EAI? */
OLDCODEp->EAIMIDIfailed = FALSE;		/* did we not succeed in opening the MIDI interface? */
OLDCODEp->EAIMIDIInitialized = FALSE; 	/* is MIDI eai running? */
#endif //OLDCODE

	}
}
void setEAIport(int pnum) {
	ppEAIServ p = (ppEAIServ)gglobal()->EAIServ.prv;
        p->EAIport = pnum;
}

void setWantEAI(int flag) {
	ppEAIServ p = (ppEAIServ)gglobal()->EAIServ.prv;
        p->EAIwanted = TRUE;
}



/* open the socket connection -  we open as a TCP server, and will find a free socket */
/* EAI will have a socket increment of 0; Java Class invocations will have 1 +	      */
int conEAIorCLASS(int socketincrement, int *EAIsockfd, int *EAIlistenfd) {
	int len;
	const int on=1;
	int flags;
#ifdef _MSC_VER
#define socklen_t int
	int err;
#endif

    struct sockaddr_in      servaddr;
	int eaiverbose;
	ppEAIServ p;
	ttglobal tg = gglobal();
	p = (ppEAIServ)tg->EAIServ.prv;
	eaiverbose = gglobal()->EAI_C_CommonFunctions.eaiverbose;

	if ((p->EAIfailed) &&(socketincrement==0)) return FALSE;

    if ((*EAIsockfd) < 0) {
		/* step 1  - create socket*/
#ifdef _MSC_VER
		static int wsaStarted;
		if(wsaStarted == 0)
			{
				WSADATA wsaData;

				/* Initialize Winsock - load correct dll */
				err = WSAStartup(MAKEWORD(2,2), &wsaData);
				if (err != 0) {
					printf("WSAStartup failed: %d\n", err);
					return FALSE;
				}
				wsaStarted = 1;
			}
#endif
	        if (((*EAIsockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf ("EAIServer: socket error\n");
#ifdef _MSC_VER
			err = WSAGetLastError();
			printf("WSAGetLastError =%d\n",err);
			if(err == WSANOTINITIALISED) printf(" WSA Not Initialized - not a successful WSAStartup\n");

#endif
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}

		setsockopt ((*EAIsockfd), SOL_SOCKET, SO_REUSEADDR, &on, (socklen_t) sizeof(on));

#ifdef _MSC_VER
		/* int ioctlsocket(SOCKET s,long cmd, u_long* argp);  http://msdn.microsoft.com/en-us/library/ms738573(VS.85).aspx */
		{
		unsigned long iMode = 1; /* nonzero is blocking */
		ioctlsocket((*EAIsockfd), FIONBIO, &iMode);
		}

#else
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
#endif
		if (eaiverbose) { 
		printf ("conEAIorCLASS - socket made\n");
		}


		/* step 2 - bind to socket*/
	        memset(&servaddr, 0, sizeof(servaddr));
	        servaddr.sin_family      = AF_INET;
	        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	        servaddr.sin_port        = htons(p->EAIport+socketincrement);
		/*printf ("binding to socket %d\n",EAIport+socketincrement);*/

	        while (bind((*EAIsockfd), (struct sockaddr *) &servaddr, (socklen_t) sizeof(servaddr)) < 0) {
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
		len = (int) sizeof(p->cliaddr);
#ifdef _MSC_VER
	        if ( ((*EAIlistenfd) = accept((*EAIsockfd), (struct sockaddr *) &p->cliaddr, (int *)&len)) < 0) {
#else
	        if ( ((*EAIlistenfd) = accept((*EAIsockfd), (struct sockaddr *) &p->cliaddr, (socklen_t *)&len)) < 0) {
#endif
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
		tg->EAIServ.EAIbufcount = 0;
		tg->EAIServ.EAIbufsize = 2 * EAIREADSIZE; /* initial size*/
		EBUFFLOCK;
		tg->EAIServ.EAIbuffer = MALLOC(char *, tg->EAIServ.EAIbufsize * sizeof (char));
		EBUFFUNLOCK;

		/* zero out the EAIListenerData here, and after every use */
		memset(&tg->EAIServ.EAIListenerData, 0, sizeof(tg->EAIServ.EAIListenerData));

		/* seems like we are up and running now, and waiting for a command */
		/* and are we using this with EAI? */
		if (socketincrement==0) p->EAIinitialized = TRUE;
#ifdef OLDCODE
OLDCODE		else if (socketincrement == MIDIPORTOFFSET) p->EAIMIDIInitialized = TRUE;
#endif //OLDCODE

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
	int eaiverbose;
	ppEAIServ p;
	ttglobal tg = gglobal();
	p = (ppEAIServ)tg->EAIServ.prv;
	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
	if (eaiverbose) { 
	printf ("shutting down EAI\n");
	}

	strcpy (tg->EAIServ.EAIListenerData,"QUIT\n\n\n");
	if (p->EAIinitialized) {
		EAI_send_string(tg->EAIServ.EAIListenerData,tg->EAIServ.EAIlistenfd);
	}

}

void fwl_create_EAI()
{
	int eaiverbose;
	ppEAIServ p;
	ttglobal tg = gglobal();
	p = (ppEAIServ)tg->EAIServ.prv;
	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
    if (eaiverbose) { 
	printf ("EAISERVER:create_EAI called\n");
	}


	/* already wanted? if so, just return */
	if (p->EAIwanted) return;

	/* so we know we want EAI */
	p->EAIwanted = TRUE;

	/* have we already started? */
	if (!p->EAIinitialized) {
		p->EAIfailed = !(conEAIorCLASS(0,&p->EAIsockfd,&tg->EAIServ.EAIlistenfd));
	}
}

#ifdef OLDCODE
OLDCODEvoid create_MIDIEAI() {
OLDCODE	int eaiverbose;
OLDCODE	ppEAIServ p;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	p = (ppEAIServ)tg->EAIServ.prv;
OLDCODE	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
OLDCODE        if (eaiverbose) {
OLDCODE        printf ("EAISERVER:create_MIDIEAI called\n");
OLDCODE        }
OLDCODE
OLDCODE        if (p->EAIMIDIwanted)  return;
OLDCODE
OLDCODE        p->EAIMIDIwanted = TRUE;
OLDCODE
OLDCODE        /* have we already started? */
OLDCODE        if (!p->EAIMIDIInitialized) {
OLDCODE                p->EAIMIDIfailed = !(conEAIorCLASS(MIDIPORTOFFSET,&p->EAIMIDIsockfd,&tg->EAIServ.EAIMIDIlistenfd));
OLDCODE        }
OLDCODE}
#endif //OLDCODE


/* possibly we have an incoming EAI request from the client */
void handle_EAI () {
	/* do nothing unless we are wanted */
	int eaiverbose;
	ppEAIServ p;
	ttglobal tg = gglobal();
	p = (ppEAIServ)tg->EAIServ.prv;
	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
	if (!p->EAIwanted) return;
	if (!p->EAIinitialized) {
		p->EAIfailed = !(conEAIorCLASS(0,&p->EAIsockfd,&tg->EAIServ.EAIlistenfd));
		return;
	}

	/* have we closed connection? */
	if (tg->EAIServ.EAIlistenfd < 0) return;

	tg->EAIServ.EAIbufcount = 0;

	EBUFFLOCK;
	tg->EAIServ.EAIbuffer = read_EAI_socket(tg->EAIServ.EAIbuffer,&tg->EAIServ.EAIbufcount, &tg->EAIServ.EAIbufsize, &tg->EAIServ.EAIlistenfd);
	/* printf ("read, EAIbufcount %d EAIbufsize %d\n",tg->EAIServ.EAIbufcount, tg->EAIServ.EAIbufsize); */

	/* make this into a C string */
	tg->EAIServ.EAIbuffer[tg->EAIServ.EAIbufcount] = 0;
	if (eaiverbose) {
		if (tg->EAIServ.EAIbufcount) printf ("handle_EAI-- Data is :%s:\n",tg->EAIServ.EAIbuffer);
	}

	/* any command read in? */
	if (tg->EAIServ.EAIbufcount > 1)
		EAI_parse_commands ();

	EBUFFUNLOCK;
}

#ifdef OLDCODE
OLDCODEvoid handle_MIDIEAI() {
OLDCODE	int eaiverbose;
OLDCODE	ppEAIServ p;
OLDCODE	ttglobal tg = gglobal();
OLDCODE	p = (ppEAIServ)tg->EAIServ.prv;
OLDCODE	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
OLDCODE
OLDCODE        if (!p->EAIMIDIwanted) return;
OLDCODE        if (!p->EAIMIDIInitialized) {
OLDCODE                p->EAIMIDIfailed = !(conEAIorCLASS(MIDIPORTOFFSET, &p->EAIMIDIsockfd, &tg->EAIServ.EAIMIDIlistenfd));
OLDCODE                return;
OLDCODE        }
OLDCODE
OLDCODE        if (tg->EAIServ.EAIMIDIlistenfd < 0) return;
OLDCODE
OLDCODE        tg->EAIServ.EAIbufcount = 0;
OLDCODE
OLDCODE	EBUFFLOCK;
OLDCODE
OLDCODE        tg->EAIServ.EAIbuffer = read_EAI_socket(tg->EAIServ.EAIbuffer, &tg->EAIServ.EAIbufcount, &tg->EAIServ.EAIbufsize, &tg->EAIServ.EAIMIDIlistenfd);
OLDCODE        /* printf ("read, MIDI EAIbufcount %d EAIbufsize %d\n",EAIbufcount, EAIbufsize); */
OLDCODE
OLDCODE        /* make this into a C string */
OLDCODE        tg->EAIServ.EAIbuffer[tg->EAIServ.EAIbufcount] = 0;
OLDCODE        if (eaiverbose) printf ("handle_EAI-- Data is :%s:\n",tg->EAIServ.EAIbuffer);
OLDCODE
OLDCODE        /* any command read in? */
OLDCODE        if (tg->EAIServ.EAIbufcount > 1)
OLDCODE                EAI_parse_commands ();
OLDCODE
OLDCODE	EBUFFUNLOCK;
OLDCODE}
#endif //OLDCODE



void EAI_send_string(char *str, int lfd){
	size_t n;
	int eaiverbose = gglobal()->EAI_C_CommonFunctions.eaiverbose;
	/* add a trailing newline */
	strcat (str,"\n");

	if (eaiverbose) {
		printf ("EAI/CLASS Command returns\n%s(end of command)\n",str);
	}

	/* printf ("EAI_send_string, sending :%s:\n",str); */
#ifdef _MSC_VER
	n = send(lfd, str, (unsigned int) strlen(str),0);
#else
	n = write (lfd, (const void *)str, strlen(str));
#endif	
	if (n<strlen(str)) {
		if (eaiverbose) {
		printf ("write, expected to write %d, actually wrote %d\n",(int)n,(int)strlen(str));
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
	int eaiverbose;
	ppEAIServ p;
	ttglobal tg = gglobal();
	p = (ppEAIServ)tg->EAIServ.prv;
	eaiverbose = tg->EAI_C_CommonFunctions.eaiverbose;
	/* if (eaiverbose) printf ("read_EAI_socket, thread %d EAIlistenfd %d buffer addr %d time %lf\n",pthread_self(),*EAIlistenfd,bf,TickTime()); */
	retval = FALSE;
	do {
		// JAS int fd;
		p->tv2.tv_sec = 0;
		p->tv2.tv_usec = 0;
		FD_ZERO(&p->rfds2);
		FD_SET((*EAIlistenfd), &p->rfds2);

		oldRetval = retval;
		retval = select((*EAIlistenfd)+1, &p->rfds2, NULL, NULL, &p->tv2);
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
#ifdef _MSC_VER
			retval = recv((*EAIlistenfd), &bf[(*bfct)],EAIREADSIZE,0);
#else
			retval = (int) read ((*EAIlistenfd), &bf[(*bfct)],EAIREADSIZE);
#endif
			if (retval <= 0) {
				if (eaiverbose) {
					printf ("read_EAI_socket, client is gone!\n");
				}

				/*perror("READ_EAISOCKET");*/
				/* client disappeared*/
#ifdef _MSC_VER
				closesocket((*EAIlistenfd));
				WSACleanup();
#else

				close ((*EAIlistenfd));
#endif
				(*EAIlistenfd) = -1;

				/* And, lets just exit FreeWRL*/
				printf ("FreeWRL:EAI socket closed, exiting...\n");
				fwl_doQuit();
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
				printf ("read_EAI_socket: HAVE TO REALLOC INPUT MEMORY:bf %p bfsz %d bfct %d\n",bf,*bfsz, *bfct);  
				(*bfsz) += EAIREADSIZE;
				/* printf ("read_EAI_socket: bfsz now %d\n",*bfsz); */
				bf = (char *)REALLOC (bf, (unsigned int) (*bfsz));
				/* printf ("read_EAI_socket: REALLOC complete\n"); */
			}
		}
	} while (retval);
	return (bf);
}


#endif //EXCLUDE_EAI
