
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
#include <stdio.h>
#ifndef WIN32
#include <sys/errno.h>
#endif

pthread_t readThread;
pthread_t swigThread;
int readThreadInitialized = FALSE;

void X3D_initialize(char *hostname) {
	struct sockaddr_in serv_addr; 
	struct hostent *server;
	int iret1;
	#ifdef WIN32
		int iret2;
	#endif
	int loopCount;
	int constat;
	int isMidi = 0;

	loopCount = 0;
	while ((_X3D_FreeWRL_FD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		usleep (100000);
		loopCount ++;
		if (loopCount >= 10000) {
			X3D_error("ERROR opening socket");
			return;
		}
	}

	usleep (10000);	/* let remote end settle down to this interruption */
	if (!strcmp(hostname, "MIDI")) {
		isMidi = 1;
		hostname = "localhost";
	}

	if (strlen(hostname) == 0) hostname = "localhost";

	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host :%s:\n",hostname);
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		 (char *)&serv_addr.sin_addr.s_addr,
		 server->h_length);
	if (isMidi) {
		serv_addr.sin_port = htons(EAIBASESOCKET + MIDIPORTOFFSET);
	} else {
		serv_addr.sin_port = htons(EAIBASESOCKET);
	}

	loopCount = 0;
	while ((constat = connect(_X3D_FreeWRL_FD,(struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {
		usleep (100000);
		loopCount ++;
		if (loopCount >= 10000) {
			X3D_error("ERROR connecting to socket - FreeWRL not there?");
			return;
		}
	}


	/* start up read thread */
	iret1 = pthread_create( &readThread, NULL, freewrlReadThread, NULL);

	/* start up thread to allow connections from SWIG (or similar) */
	/* iret2 = pthread_create(&swigThread, NULL, freewrlSwigThread, NULL); */
#ifdef WIN32
	iret2 = pthread_create(&swigThread, NULL, freewrlSwigThread, NULL); 
#endif
}



/* tell FreeWRL to shut down; don't worry about the return value */
void X3D_shutdown() {
	_X3D_makeShortCommand(STOPFREEWRL);
#ifdef WIN32
	/* cleanup I suspect it never gets here */
    closesocket(_X3D_FreeWRL_FD);
    WSACleanup();
#endif
}
