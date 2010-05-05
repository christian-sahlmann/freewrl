
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


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "system.h"
#include "soundheader.h"


/* record "global" parameters */
long int MaxSPS = 0;
int MaxAvgBytes = 0;
int myBPS;

void streamThisChannel(int source, int bytesToStream, int offset) {
	SNDFILE *wavfile;
	int readSizeThrowAway;

	/* //printf ("streamThisChannel %d, bytes %d offset %d\n",source, */
	/* //		bytesToStream, offset); */

	wavfile = sndfile[source];

	/* // ok - we should write. Get the next bit of data */

	/* // Calculate if we are going to go past the EOF marker, */
	/* // and if so, go back to the beginning. (assume loop=true) */
	/* // */
	/* //printf ("wavfile->bytes_remaining %d\n",wavfile->bytes_remaining); */
	/* // has this file been successfully initialized yet? */
	if (wavfile->bytes_remaining == UNINITWAV) {
		/* //printf ("file not opened yet\n"); */
		rewind_to_beginning(wavfile);
	}

	if (wavfile->bytes_remaining <= 0) {
		/* //printf ("EOF input, lets reset and re-read\n"); */
		if (loop[source] == 1) {
			rewind_to_beginning(wavfile);
		} else {
			/* // dont loop - just return */
			return;
		}
	}

	/* //printf ("bytes remaining %ld\n",wavfile->bytes_remaining); */

	/* // Are we reaching the end of the file? Lets calculate the */
	/* // size of the WAV file read we are going to do, and adjust */
	/* // where we are in the read of the WAV file. Note that for */
	/* // really short files, this works, too! */
	/* // */
	/* // */

	/* // */
	/* //printf ("step2, bytes remaining %ld\n",wavfile->bytes_remaining); */
	if (wavfile->bytes_remaining < bytesToStream) {
		readSize = (int) (wavfile->bytes_remaining);
		wavfile->bytes_remaining = 0;
	} else {
		readSize = bytesToStream;
		wavfile->bytes_remaining = wavfile->bytes_remaining -
			(long int) bytesToStream;
	}
	/* //printf ("after decrement, %ld\n", wavfile->bytes_remaining); */

	/* // read and write here. */
	if ((readSize) > 0) {
		/* //printf ("reading/writing %d bytes\n",readSize); */
		readSizeThrowAway = fread(wavfile->data,readSize,1,wavfile->fd);
		addToCombiningBuffer(source,readSize,offset);
	}

	/* // do we need to read some more? */
	if (readSize < bytesToStream) {
		streamThisChannel(source, bytesToStream-readSize,
						  offset+readSize);
	}
}



void streamMoreData(int bytesToStream) {
	int count;
	int writeSizeThrowAway;

	if (bytesToStream > MAXBUFSIZE) {bytesToStream = MAXBUFSIZE;
		/* //printf ("reducing bytesToStream\n"); */
	}

	for (count = 0; count <= current_max; count++) {
		if ((active[count] == 1 ) && (registered[count] == 1)) {
			/* //printf ("channel %d is active\n",count); */
			if ((sndfile[count]->ampl) > 0) {
				streamThisChannel(count,bytesToStream,0);
			}

			/* // Now, did we loose this source? lets decrement the */
			/* // amplitude, until we get either to zero, or another */
			/* // AMPL command. */

			if (sndfile[count]!=NULL) {
			    sndfile[count]->ampl = (sndfile[count]->ampl) - 5;
			    if ((sndfile[count]->ampl) <= 0) {
					sndfile[count]->ampl = 0;
			    }
			}
		}
	}
	/* // Now, stream out the combined data */
	writeSizeThrowAway = write (dspFile, CombiningBuffer, bytesToStream);
}
