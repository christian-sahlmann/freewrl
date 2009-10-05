
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

#ifndef REWIRE
#include "config.h"
#include "system.h"
#endif
#include "EAI_C.h"

#define WAIT_FOR_RETVAL ((command!=SENDEVENT) && (command!=MIDICONTROL))

static pthread_mutex_t eailock = PTHREAD_MUTEX_INITIALIZER;
#define EAILOCK pthread_mutex_lock(&eailock);
#define EAIUNLOCK pthread_mutex_unlock(&eailock);

int _X3D_FreeWRL_FD;
int _X3D_FreeWRL_Swig_FD = 0;
int _X3D_FreeWRL_listen_FD = 0;
int _X3D_queryno = 1;

int receivedData= FALSE;
/* for waiting on a socket */
fd_set rfds;
struct timeval tv;
struct timeval tv2;

void X3D_error(char *msg) {
    perror(msg);
    exit(0);
}

double mytime;

char readbuffer[2048];
char *sendBuffer = NULL;
int sendBufferSize = 0;

/* handle a callback - this should get a line like:
 EV
1170697988.125835
31
0.877656
EV_EOT
*/

/* make a buffer large enough to hold our data */
void verifySendBufferSize (int len) {
	if (len < (sendBufferSize-50)) return;
	
	/* make it large enough to contain string, plus some more, as we usually throw some stuff on the beginning. */
	while (len>(sendBufferSize-200)) sendBufferSize+=1024;
	sendBuffer = realloc(sendBuffer,sendBufferSize);
}

/* count the number of numbers on a line - useful for MFNode return value mallocs */
int _X3D_countWords(char *ptr) {
	int ct;
	
	ct = 0;
	
	while (*ptr >= ' ') {
		SKIP_CONTROLCHARS
		SKIP_IF_GT_SPACE
		ct ++;
	}
	return ct;
}
#ifdef WIN32
void *freewrlSwigThread(void* nada) {
#else
void freewrlSwigThread(void) {
#endif
	const int on=1;
	int flags;
	int len;

	struct sockaddr_in servaddr, cliaddr;
#ifdef WIN32
    int iResult;
    WSADATA wsaData;

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        X3D_error("WSAStartup failed to load winsock2 ws2_32.dll\n");
        return NULL;
    }
#endif

	if ((_X3D_FreeWRL_listen_FD= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
              X3D_error("ERROR opening swig socket");
              return NULL;
        }
	
	setsockopt(_X3D_FreeWRL_listen_FD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(EAIBASESOCKET+ 500);
	
	if (bind((_X3D_FreeWRL_listen_FD), (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		X3D_error("ERROR in bind");
	}

	if (listen(_X3D_FreeWRL_listen_FD, 1024) < 0) {
		X3D_error("ERROR in listen");
		return NULL;
	}

	len = sizeof(cliaddr);

#ifdef WIN32
	_X3D_FreeWRL_Swig_FD = accept((_X3D_FreeWRL_listen_FD), (struct sockaddr*) &cliaddr, &len);
#else
	_X3D_FreeWRL_Swig_FD = accept((_X3D_FreeWRL_listen_FD), (struct sockaddr*) &cliaddr, (socklen_t *) &len);
#endif
	return NULL;
}

/* read in the reply - if it is an RE; it is the reply to an event; if it is an
   EV it is an async event */

#ifdef WIN32
void* freewrlReadThread(void *nada)  {
#else
void freewrlReadThread(void)  {
#endif
	int retval;
	while (1==1) {


                tv.tv_sec = 0;
                tv.tv_usec = 100;
                FD_ZERO(&rfds);
                FD_SET(_X3D_FreeWRL_FD, &rfds);

                /* wait for the socket. We HAVE to select on "sock+1" - RTFM */
				/* WIN32 ignors the first select parameter, just there for berkley compatibility */
                retval = select(_X3D_FreeWRL_FD+1, &rfds, NULL, NULL, &tv);

                if (retval) {
#ifdef WIN32
				retval = recv(_X3D_FreeWRL_FD, readbuffer, 2048, 0);
				if (retval  == SOCKET_ERROR) {
#else
			retval = read(_X3D_FreeWRL_FD,readbuffer,2048);
				if (retval  <= 0) {
#endif
					printf("ERROR reading fromsocket\n");
					exit(1);
				}
			readbuffer[retval] = '\0';

			/* if this is normal data - signal that it is received */
			if (strncmp ("RE",readbuffer,2) == 0) {
				receivedData = TRUE;
			} else if (strncmp ("EV",readbuffer,2) == 0) {
					_handleFreeWRLcallback(readbuffer);
			} else if (strncmp ("RW",readbuffer,2) == 0) {
				_handleReWireCallback(readbuffer);
			} else if (strncmp ("QUIT",readbuffer,4) == 0) {
				exit(0);
			} else {
				printf ("readThread - unknown prefix - %s\n",readbuffer);
			}

		/*
                } else {
                                printf ("waitForData, timing out\n","");
		*/

                }

	}
}

/* threading - we thread only to read from a different thread. This
allows events and so on to go quickly - no return value required. */

static char *sendToFreeWRL(char *callerbuffer, int size, int waitForResponse) {
	int retval;
	int readquery;
	char *ptr;

	#ifdef VERBOSE
	printf ("sendToFreeWRL - sending :%s:\n",callerbuffer);
	#endif

#ifdef WIN32
	ptr = NULL;
	retval = send(_X3D_FreeWRL_FD, callerbuffer, size, 0);
	if (retval == SOCKET_ERROR ) 
#else	
	retval = write(_X3D_FreeWRL_FD, callerbuffer, size);
	if (retval < 0) 
#endif
		 X3D_error("ERROR writing to socket");

	if (waitForResponse) {

		receivedData = FALSE;
		while (!receivedData) {
			sched_yield();
		}


		/* have the response here now. */

		#ifdef VERBOSE
		printf("Client got: %s\n",readbuffer);
		#endif
		
		/* should return something like: RE
1165347857.925786
1
0.000000
RE_EOT
*/
		/* see if it is a reply, or an event return */


		ptr = readbuffer;
		while ((*ptr != '\0') && (*ptr <= ' ')) ptr++;

		#ifdef VERBOSE
		printf ("found a reply\n");
		#endif

		SKIP_IF_GT_SPACE
		SKIP_CONTROLCHARS
		if (sscanf(ptr,"%lf",&mytime) != 1) {
			printf ("huh, expected the time, got %s\n",ptr);
			exit(1);
		}
		#ifdef VERBOSE
		printf ("time of command is %lf\n",mytime);
		#endif

		SKIP_IF_GT_SPACE
		SKIP_CONTROLCHARS

		#ifdef VERBOSE
		printf ("this should be the query number: %s\n",ptr);
		#endif

		if (sscanf(ptr,"%d",&readquery) != 1) {
			printf ("huh, expected the time, got %s\n",ptr);
			exit(1);
		}
		#ifdef VERBOSE
		printf ("query returned is %d\n",readquery);
		#endif

		if (_X3D_queryno != readquery) {
			printf ("server: warning, _X3D_queryno %d != received %d\n",_X3D_queryno,readquery);
			usleep(5000);
			sched_yield();
		}

		SKIP_IF_GT_SPACE
		SKIP_CONTROLCHARS


		strncpy(callerbuffer,readbuffer,retval);

	}
	_X3D_queryno ++;
	return ptr;
}


void _X3D_sendEvent (char command, char *string) {
        char *myptr;
	EAILOCK
	verifySendBufferSize (strlen(string));
        sprintf (sendBuffer, "%d %c %s\n",_X3D_queryno,command,string);
        myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),WAIT_FOR_RETVAL);
	EAIUNLOCK
}

char *_X3D_makeShortCommand (char command) {
	char *myptr;

	EAILOCK
	verifySendBufferSize (100);
	sprintf (sendBuffer, "%d %c\n",_X3D_queryno,command);
	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),WAIT_FOR_RETVAL);
	EAIUNLOCK
	#ifdef VERBOSE
	printf ("makeShortCommand, buffer now %s\n",myptr);
	#endif
	return myptr;
}

char *_X3D_make1VoidCommand (char command, uintptr_t *adr) {
	char *myptr;

	EAILOCK
	verifySendBufferSize (100);
	sprintf (sendBuffer, "%d %c %p\n",_X3D_queryno,command,adr);
	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),WAIT_FOR_RETVAL);
	EAIUNLOCK
	#ifdef VERBOSE
	printf ("make1VoidCommand, buffer now %s\n",myptr);
	#endif
	return myptr;
}

char *_X3D_make1StringCommand (char command, char *name) {
	char *myptr;
	
	EAILOCK
	verifySendBufferSize (strlen(name));
	sprintf (sendBuffer, "%d %c %s\n",_X3D_queryno,command,name);
	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),WAIT_FOR_RETVAL);
	EAIUNLOCK
	#ifdef VERBOSE
	printf ("make1StringCommand, buffer now %s\n",myptr);
	#endif
	return myptr;
}

char *_X3D_make2StringCommand (char command, char *str1, char *str2) {
	char *myptr;
	char sendBuffer[2048];
	
	EAILOCK
	verifySendBufferSize ( strlen(str1) + strlen(str2));
	sprintf (sendBuffer, "%d %c %s%s\n",_X3D_queryno,command,str1,str2);
	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),WAIT_FOR_RETVAL);
	EAIUNLOCK

	#ifdef VERBOSE
	printf ("make2StringCommand, buffer now %s\n",myptr);
	#endif
	return myptr;
}


char *_X3D_Browser_SendEventType(uintptr_t *adr,char *name, char *evtype) {
	char *myptr;

	EAILOCK
	verifySendBufferSize (100);
	sprintf (sendBuffer, "%u %c 0 %d %s %s\n",_X3D_queryno, GETFIELDTYPE, (unsigned int) adr, name, evtype);

	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),TRUE);
	EAIUNLOCK
	#ifdef VERBOSE
	printf ("_X3D_Browser_SendEventType, buffer now %s\n",myptr);
	#endif
	return myptr;
}

char * _RegisterListener (X3DEventOut *node, int adin) {
	char *myptr;
	

	verifySendBufferSize (100);
	#ifdef VERBOSE
	printf ("in RegisterListener, we have query %d advise index %d nodeptr %d offset %d datatype %d datasize %d field %s\n",
		_X3D_queryno,
                adin, node->nodeptr, node->offset, node->datatype, node->datasize, node->field);
	#endif

/*
 EAIoutSender.send ("" + queryno + "G " + nodeptr + " " + offset + " " + datatype +
                " " + datasize + "\n");
*/
	EAILOCK
	sprintf (sendBuffer, "%u %c %ld %d %c %d\n",
		_X3D_queryno, 
		REGLISTENER, 
		node->nodeptr,
		node->offset,
		mapFieldTypeToEAItype(node->datatype),
		node->datasize);

	myptr = sendToFreeWRL(sendBuffer, strlen(sendBuffer),TRUE);
	EAIUNLOCK
	#ifdef VERBOSE
	printf ("_X3D_Browser_SendEventType, buffer now %s\n",myptr);
	#endif
	return myptr;
}

