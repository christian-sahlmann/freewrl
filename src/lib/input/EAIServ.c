/*
=INSERT_TEMPLATE_HERE=

$Id: EAIServ.c,v 1.29 2012/07/08 15:17:45 dug9 Exp $

Implement Socket server functionality for FreeWRL.
This is currently (Jun 2012) used by the EAI and the MIDI routines

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

/* #include <display.h> */
#include <internal.h>
#include <libFreeWRL.h>

/* !! This stuff is private. You should only use the functions defined in libFreeWRL.h
 * or the constants defined in SCKHeaders.h
 */

#include "SCKHeaders.h"

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
/************************************************************************/

/* static pthread_mutex_t sckbufferlock = PTHREAD_MUTEX_INITIALIZER; */

/* State */
int service_connected[MAX_SERVICE_CHANNELS] ;
int service_failed[MAX_SERVICE_CHANNELS] ;
int service_onclose[MAX_SERVICE_CHANNELS] ;
int service_wanted[MAX_SERVICE_CHANNELS] ;
int service_verbose[MAX_SERVICE_CHANNELS] ;
int  service_status[MAX_SERVICE_CHANNELS] ;
/* More defined in fwlio_RxTx_control as static ints */

unsigned char loopFlags = 0;

enum theLoopFlags {
		NO_CLIENT_CONNECTED = 0x1,
		NO_EAI_CLASS	    = 0x2,
		NO_RETVAL_CHANGE    = 0x4
};

/* socket stuff */
struct sockaddr_in	servaddr, cliaddr;
#define MAINSOCK_FD 0
#define CLIENT_FD 1
int SCK_descriptors[MAX_SERVICE_CHANNELS][2] ;
int SCK_port[MAX_SERVICE_CHANNELS] ;

/* Buffer IO */
char *sock_buffers[MAX_SERVICE_CHANNELS] ;
int sock_bufcount[MAX_SERVICE_CHANNELS] ;
int sock_bufsize[MAX_SERVICE_CHANNELS] ;
fd_set rfds2;
struct timeval tv2;

/* Low level private functions */
int privSocketSetup(int channel, int *sockfd, int *listenfd);
char *privSocketRead(int channel, char *bf, int *bfct, int *bfsz, int *listenfd);

/* **************************************************************
 * Public top level interface
 * ************************************************************** */

void create_MIDIEAI() {
	int result ;
	result = fwlio_RxTx_control(CHANNEL_MIDI, RxTx_START) ;
}

int fwlio_RxTx_control(int channel, int action) {
	static int first_time = 1 ;
	static int service_status[MAX_SERVICE_CHANNELS] ;
	static int service_justSink[MAX_SERVICE_CHANNELS] ;

	if (service_verbose[channel] > 1) { 
		printf ("fwlio_RxTx_control(%d,...) START: called with action code %d\n", channel,action);
		printf ("fwlio_RxTx_control: service_status[i] = %d\n", service_status[channel]) ;
		printf ("fwlio_RxTx_control: service_justSink[i] = %d\n", service_justSink[channel]) ;
		printf ("fwlio_RxTx_control: service_wanted[i] = %d\n", service_wanted[channel]) ;
		printf ("fwlio_RxTx_control: service_connected[i] = %d\n", service_connected[channel]) ;
		printf ("fwlio_RxTx_control: service_failed[i] = %d\n", service_failed[channel]) ;
		printf ("fwlio_RxTx_control: service_onclose[i] = %d\n", service_onclose[channel]) ;
		printf ("fwlio_RxTx_control: SCK_descriptors[i][0] = %d\n", SCK_descriptors[channel][0]) ;
		printf ("fwlio_RxTx_control: SCK_descriptors[i][1] = %d\n", SCK_descriptors[channel][1]) ;
		printf ("fwlio_RxTx_control: SCK_port[i] = %d\n", SCK_port[channel]) ;
	}
	if (first_time != 0 ) {
		int i;
		for (i=0; i<MAX_SERVICE_CHANNELS; i++) {
			service_status[i] = RxTx_STOP ;
			service_justSink[i] = 0 ;
			service_wanted[i] = FALSE;
			service_connected[i] = FALSE;
			service_failed[i] = FALSE;
			service_onclose[i] = FALSE ;
			service_verbose[i] = SOCKET_SERVICE_DEFAULT_VERBOSE ;
			SCK_descriptors[i][0] = -1 ;
			SCK_descriptors[i][1] = -1 ;
			SCK_port[i] = -1 ;
		}
		SCK_port[CHANNEL_EAI] = EAIBASESOCKET ;
		SCK_port[CHANNEL_MIDI] = EAIBASESOCKET + MIDIPORTOFFSET ;
	}
	first_time = 0 ;

	/*
	 * In general, this function will just return the original action code, except:
	 * It will return 0 if the action was not a valid state request,
	 * It will return N if the control action requires that (for example the state code)
	 */
	if( RxTx_STOP == action && service_status[channel] != RxTx_STOP) {
		if (service_verbose[channel]) { 
			printf ("Shutting down the socket on logical channel %d\n",channel);
		}

		if (service_connected[channel] && channel == CHANNEL_EAI) {
			fwlio_RxTx_sendbuffer(__FILE__,__LINE__,channel, "QUIT\n\n\n");
		}
		service_status[channel]=RxTx_STOP;
	}
	if( RxTx_START == action) {

		/* already wanted? if so, just return */
		if (service_wanted[channel] && service_status[channel] != RxTx_STOP) return RxTx_START;

		/* so we know we want EAI */
		service_wanted[channel] = TRUE;

		/* have we already started? */
		if (!service_connected[channel]) {
			int EAIsockfd = -1;	/* main TCP socket fd*/
			int EAIlistenfd = -1;	/* listen to this one for an incoming connection*/
			service_failed[channel] = !(privSocketSetup(channel, &EAIsockfd, &EAIlistenfd));
			if(service_failed[channel]) {
				return 0;
			} else {
				SCK_descriptors[channel][MAINSOCK_FD] = EAIsockfd ;
				SCK_descriptors[channel][CLIENT_FD] = EAIlistenfd ;
				service_status[channel] = RxTx_REFRESH;
			}
		}
	}
	if( RxTx_REFRESH == action) {
		int EAIsockfd, EAIlistenfd;
		int E_SOCK_bufcount;
		int E_SOCK_bufsize;
		char *E_SOCK_buffer; 

		if (!service_wanted[channel]) return 0;
		EAIsockfd =  SCK_descriptors[channel][MAINSOCK_FD] ;
		EAIlistenfd = SCK_descriptors[channel][CLIENT_FD] ;
		if (!service_connected[channel]) {
			service_failed[channel] = !(privSocketSetup(channel,&EAIsockfd,&EAIlistenfd));
			if(service_failed[channel]) {
				return 0;
			} else {
				SCK_descriptors[channel][CLIENT_FD] = EAIlistenfd ;
				service_status[channel] = RxTx_REFRESH;
			}
		}
		if (!service_connected[channel] > 1) {
			if (service_verbose[channel]) { 
				printf ("Still no client connection on channel %d\n",channel);
			}
			return 0;
		}

		/* have we closed connection? */
		if(SCK_descriptors[channel][CLIENT_FD] < 0) return 0;

		E_SOCK_bufcount = sock_bufcount[channel];
		E_SOCK_bufsize = sock_bufsize[channel] ;
		E_SOCK_buffer = sock_buffers[channel] ; 

		/* EBUFFLOCK; */
		sock_buffers[channel] = privSocketRead(channel,E_SOCK_buffer,&E_SOCK_bufcount, &E_SOCK_bufsize, &EAIlistenfd);
		if (service_verbose[channel] > 1) {
			printf ("Buffer(%d) addr %p\n",channel,sock_buffers[channel] );
		}
		/* printf ("read, E_SOCK_bufcount %d E_SOCK_bufsize %d\n",E_SOCK_bufcount, E_SOCK_bufsize); */
		if(service_justSink[channel] == 1) E_SOCK_bufcount = 0;
		sock_bufcount[channel]  = E_SOCK_bufcount ;
		sock_bufsize[channel] = E_SOCK_bufsize ;

		/* make this into a C string */
		sock_buffers[channel][sock_bufcount[channel]] = 0;
		/* EBUFFUNLOCK; */
		if (service_verbose[channel] > 0) { 
			if (sock_bufcount[channel]) {
				printf ("fwlio_RxTx_control: sock_bufcount[%d] = %d\nData is :%s:\n\n",
				channel,
				sock_bufcount[channel],
				sock_buffers[channel]);
			}
		}
		return sock_bufcount[channel] ;

	}
	if( RxTx_STOP_IF_DISCONNECT == action) {
		service_onclose[channel] = TRUE ;
	}
	if( RxTx_EMPTY == action) {
		sock_bufcount[channel] = 0;
		sock_buffers[channel][sock_bufcount[channel]] = 0;
	}
	if( RxTx_MOREVERBOSE == action) {
		service_verbose[channel] += 1 ;
	}
	if( RxTx_GET_VERBOSITY == action) {
		return service_verbose[channel] ;
	}
	if( RxTx_SILENT == action) {
		service_verbose[channel] = 0 ;
	}
	if( RxTx_SINK == action) {
		service_justSink[channel] = 1;
	}
	if( RxTx_PENDING == action) {
		/* we might make this a bit more flexble */
		if (service_verbose[channel] > 0) { 
			printf ("fwlio_RxTx_control: sock_bufcount[%d] (addr %p) = %d\n",
				channel, sock_buffers[channel], sock_bufcount[channel]) ;
		}
		return sock_bufcount[channel] ;
	}
	if( RxTx_STATE == action) {
		return service_status[channel] ;
	}
	if (service_verbose[channel] > 1) { 
		printf ("fwlio_RxTx_control() END:\n");
		printf ("fwlio_RxTx_control: service_status[i] = %d\n", service_status[channel]) ;
		printf ("fwlio_RxTx_control: service_justSink[i] = %d\n", service_justSink[channel]) ;
		printf ("fwlio_RxTx_control: service_wanted[i] = %d\n", service_wanted[channel]) ;
		printf ("fwlio_RxTx_control: service_connected[i] = %d\n", service_connected[channel]) ;
		printf ("fwlio_RxTx_control: service_failed[i] = %d\n", service_failed[channel]) ;
		printf ("fwlio_RxTx_control: service_onclose[i] = %d\n", service_onclose[channel]) ;
		printf ("fwlio_RxTx_control: SCK_descriptors[i][0] = %d\n", SCK_descriptors[channel][0]) ;
		printf ("fwlio_RxTx_control: SCK_descriptors[i][1] = %d\n", SCK_descriptors[channel][1]) ;
		printf ("fwlio_RxTx_control: SCK_port[i] = %d\n", SCK_port[channel]) ;
		printf ("fwlio_RxTx_control: sock_bufcount[%d] (addr %p) = length %d\n\n",
			channel, sock_buffers[channel], sock_bufcount[channel]) ;
	}

	return action ;
}

char * fwlio_RxTx_waitfor(int channel, char *str) {
	return strstr(sock_buffers[channel], str);
}

char *fwlio_RxTx_getbuffer(int channel) {

	char *tmpStr = MALLOC(char *, sock_bufcount[channel] + 1);
	if (service_verbose[channel]) {
		printf ("fwlio_RxTx_getbuffer(%d)\n", channel);
		printf ("fwlio_RxTx_getbuffer: Copy %d chars in buffer(%d) from addr %p to %p\n",sock_bufcount[channel],channel,sock_buffers[channel] ,tmpStr );
	}

	if (!tmpStr)
		return NULL;

	memcpy(tmpStr, sock_buffers[channel], sock_bufcount[channel]);
	tmpStr[sock_bufcount[channel]] = '\0';
	memset(sock_buffers[channel], 0, sock_bufcount[channel]);
	sock_bufcount[channel] = 0 ;

	if (service_verbose[channel]) {
		printf ("fwlio_RxTx_getbuffer: return %s\n\n",tmpStr );
	}
	return tmpStr;
}

void fwlio_RxTx_sendbuffer(char *fromFile, int fromline, int channel, char *str) {
	size_t n;

	/* add a trailing newline */
	strcat (str,"\n");

	if (service_verbose[channel]) {
		printf ("fwlio_RxTx_sendbuffer(%s,%d,%d,..), sending :\n%s\n(on FD %d)\n",fromFile,fromline,channel,str,SCK_descriptors[channel][CLIENT_FD]);
	}

#ifdef _MSC_VER
	n = send(SCK_descriptors[channel][CLIENT_FD], str, (unsigned int) strlen(str),0);
#else
	/* n = write (SCK_descriptors[channel][CLIENT_FD], (const void *)str, strlen(str)); */
	n = write (SCK_descriptors[channel][CLIENT_FD], str, strlen(str));
#endif	
	if (service_verbose[channel]) {
		printf ("fwlio_RxTx_sendbuffer(...%d,..), wrote %d 'chars'\n",channel,(int)n);
	}
	if (n<strlen(str)) {
		printf ("fwlio_RxTx_sendbuffer(...%d,..) write, expected to write %d, actually wrote %d\n",channel,(int)n,(int)strlen(str));
	}
}

/* **************************************************************
 * Private low level funtions
 * ************************************************************** */

/* open the socket connection -  we open as a TCP server, and will find a free socket */
/* EAI will have a socket increment of 0; Java Class invocations will have 1 +	      */
int privSocketSetup(int channel, int *ANONsocketfd, int *ANONlistenfd) {
	int len;
	const int on=1;
	int flags;
#ifdef _MSC_VER
#define socklen_t int
	int err;
#endif

        struct sockaddr_in      servaddr;

	/* if ((service_failed[channel]) && (channel==0)) return FALSE; */
	if ((service_failed[channel])) return FALSE;

	if ((*ANONsocketfd) < 0) {
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
	        if (((*ANONsocketfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf ("WRL_Server: socket error\n");
#ifdef _MSC_VER
			err = WSAGetLastError();
			printf("WSAGetLastError =%d\n",err);
			if(err == WSANOTINITIALISED) printf(" WSA Not Initialized - not a successful WSAStartup\n");

#endif
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}

		setsockopt ((*ANONsocketfd), SOL_SOCKET, SO_REUSEADDR, &on, (socklen_t) sizeof(on));

#ifdef _MSC_VER
		/* int ioctlsocket(SOCKET s,long cmd, u_long* argp);  http://msdn.microsoft.com/en-us/library/ms738573(VS.85).aspx */
		{
		unsigned long iMode = 1; /* nonzero is blocking */
		ioctlsocket((*ANONsocketfd), FIONBIO, &iMode);
		}

#else
		if ((flags=fcntl((*ANONsocketfd),F_GETFL,0)) < 0) {
			printf ("EAIServer: trouble gettingsocket flags\n");
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		} else {
			flags |= O_NONBLOCK;

			if (fcntl((*ANONsocketfd), F_SETFL, flags) < 0) {
				printf ("EAIServer: trouble setting non-blocking socket\n");
				loopFlags &= ~NO_EAI_CLASS;
				return FALSE;
			}
		}
#endif
		if (service_verbose[channel]) { 
			printf ("privSocketSetup - socket made\n");
		}


		/* step 2 - bind to socket*/
	        memset(&servaddr, 0, sizeof(servaddr));
	        servaddr.sin_family      = AF_INET;
	        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	        servaddr.sin_port        = htons(SCK_port[channel]);
		if (service_verbose[channel]) { 
			printf ("FreeWRL socket server binding to socket on port %d for channel %d\n",SCK_port[channel], channel);
		}

	        while (bind((*ANONsocketfd), (struct sockaddr *) &servaddr, (socklen_t) sizeof(servaddr)) < 0) {
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}

		if (service_verbose[channel]) { 
			printf ("FreeWRL socket server: bound to socket %d\n",SCK_port[channel]);
		}


		/* step 3 - listen*/

	        if (listen((*ANONsocketfd), 1024) < 0) {
	                printf ("FreeWRL socket server: listen error\n");
			loopFlags &= ~NO_EAI_CLASS;
			return FALSE;
		}
	}
	if (service_verbose[channel]) { 
		if ( (*ANONsocketfd) >= 0 ) {
			if (service_verbose[channel] > 1) { 
				printf("We have a valid socket fd, %d",(*ANONsocketfd)) ;
			}
		}
		if ( (*ANONlistenfd) >= 0 ) {
			printf(" and we have a client connected fd, %d\n",(*ANONlistenfd)) ;
		} else {
			if (service_verbose[channel] > 1) { 
				printf(" but no client connection (yet)\n") ;
			}
		}
	}

	if (((*ANONsocketfd) >=0 ) && ((*ANONlistenfd)<0)) {
		/* step 4 - accept*/
		if (service_verbose[channel]>1) { 
			printf("We are going to attempt a non-blocking accept()..\n") ;
		}
		len = (int) sizeof(cliaddr);
#ifdef _MSC_VER
	        if ( ((*ANONlistenfd) = accept((*ANONsocketfd), (struct sockaddr *) &cliaddr, (int *)&len)) < 0) {
#else
	        if ( ((*ANONlistenfd) = accept((*ANONsocketfd), (struct sockaddr *) &cliaddr, (socklen_t *)&len)) < 0) {
#endif
			if (service_verbose[channel]>1) {
				if (!(loopFlags&NO_CLIENT_CONNECTED)) {
					printf ("FreeWRL socket server: no client yet\n");
					loopFlags |= NO_CLIENT_CONNECTED;
				}
			}

		} else {
			loopFlags &= ~NO_CLIENT_CONNECTED;
			if (service_verbose[channel]) {
				printf ("FreeWRL socket server: we have a client\n");
			}
		}
	}


	/* are we ok, ? */
	if ((*ANONlistenfd) >=0)  {
		if(!service_connected[channel]) {
			SCK_descriptors[channel][1] = (*ANONlistenfd) ;
			/* allocate memory for input buffer */
			sock_bufcount[channel] = 0;
			sock_bufsize[channel] = 2 * EAIREADSIZE; /* initial size*/
			if (service_verbose[channel]) {
				printf ("FreeWRL socket server: malloc a buffer,%d\n",sock_bufsize[channel]);
			}
			/* EBUFFLOCK; */
			sock_buffers[channel] = MALLOC(char *, sock_bufsize[channel] * sizeof (char));
			/* EBUFFUNLOCK; */

			if(channel == CHANNEL_EAI) {
				if (service_verbose[channel]) {
					printf("Go and clear the listener node\n") ;
				}
				fwl_EAI_clearListenerNode();
			}

			/* seems like we are up and running now, and waiting for a command */
			service_connected[channel] = TRUE;
		} else {
			printf ("FreeWRL socket server: Why are we in %s,%d? we should not be here!!\n",__FILE__,__LINE__);
		}
	}
	/* printf ("FreeWRL socket server: privSocketSetup returning TRUE\n");*/

	if (service_verbose[channel]) {
		if ( !(loopFlags&NO_EAI_CLASS)) {
			printf ("FreeWRL socket server: privSocketSetup returning TRUE\n");
			loopFlags |= NO_EAI_CLASS;
		}
	}

	return TRUE;
}

/* read in from the socket.   pass in -
	pointer to buffer,
	pointer to buffer index
	pointer to max size,
	pointer to socket to listen to

 	return the char pointer - it may have been REALLOC'd */


char *privSocketRead(int channel, char *bf, int *bfct, int *bfsz, int *EAIlistenfd) {
	int retval, oldRetval;

	if (service_verbose[channel] > 1) {
		printf ("privSocketRead (polling), listenfd %d, buffer addr %p\n",(*EAIlistenfd),(void *) bf);
	}
	retval = FALSE;
	do {
		tv2.tv_sec = 0;
		tv2.tv_usec = 0;
		FD_ZERO(&rfds2);
		FD_SET((*EAIlistenfd), &rfds2);

		oldRetval = retval;
		retval = select((*EAIlistenfd)+1, &rfds2, NULL, NULL, &tv2);
		if (service_verbose[channel] > 1) printf ("select retval %d\n",retval);

		if (retval != oldRetval) {
			loopFlags &= NO_RETVAL_CHANGE;
		}

		if (service_verbose[channel] > 1) {
			if (!(loopFlags&NO_RETVAL_CHANGE)) {
				printf ("privSocketRead, retval %d\n",retval);
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
				if (service_verbose[channel]) {
					printf ("privSocketRead, client is gone!\n");
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
				SCK_descriptors[channel][0] = -1 ;
				SCK_descriptors[channel][1] = -1 ;
				service_status[channel] = RxTx_STOP ;
				service_wanted[channel] = FALSE;
				service_connected[channel] = FALSE;
				service_failed[channel] = FALSE;

				if(service_onclose[channel] == TRUE) {
					/* And, lets just exit FreeWRL*/
					printf ("FreeWRL:EAI socket closed, exiting...\n");
					fwl_doQuit();
					return (bf);
				} else {
					return (bf);
				}
			}

			if (service_verbose[channel]>1) {
			    char tmpBuff1[EAIREADSIZE];
			    strncpy(tmpBuff1,&bf[(*bfct)],retval);
			    tmpBuff1[retval] = '\0';
			    printf ("privSocketRead %d bytes, max %d bfct %d input=<%s>\n",
				    retval,EAIREADSIZE, *bfct,tmpBuff1);/*, &bf[(*bfct)]);*/
			}


			(*bfct) += retval;

			if (((*bfsz) - (*bfct)) <= EAIREADSIZE) {
				if (service_verbose[channel]) 
					printf ("privSocketRead: HAVE TO REALLOC INPUT MEMORY:bf %p bfsz %d bfct %d\n",bf,*bfsz, *bfct);  
				(*bfsz) += EAIREADSIZE;
				if (service_verbose[channel]) 
					printf ("privSocketRead: bfsz now %d\n",*bfsz);
				bf = (char *)REALLOC (bf, (unsigned int) (*bfsz));
				if (service_verbose[channel]) 
					printf ("privSocketRead: REALLOC complete\n");
			}
		}
		if (service_verbose[channel] > 1) {
			printf ("Buffer addr %p\n",(void *) bf);
		}
	} while (retval);
	return (bf);
}
#endif //EXCLUDE_EAI

