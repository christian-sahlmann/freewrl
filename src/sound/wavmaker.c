/*******************************************************************
 *  Copyright (C) 2002 John Stewart, CRC Canada.
 *  DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 *  See the GNU Library General Public License (file COPYING in the distribution)
 *  for conditions of use and redistribution.
 *
 *******************************************************************/

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include "system.h"
#include "soundheader.h"


int 	dspFile = -1;		/*  Sound output device*/
char 	*dspBlock = NULL;	/*  a block to send*/

/*  Fragment parameters*/
int readSize;			/*  how much to read from wav file - either BUFSIZE or less*/

/*  are we playing a sound?*/
int DSPplaying = -1;

/*  How many bytes have been used since the last cycle?*/
int bytesPerCycle = 0;

/*  how many bytes can we write to the sound card assuming no sound playing?*/
int soundcardBufferEmptySize = 0;
int soundcardBufferCurrentSize = 0;

/*  DSP parameters, should match sox call parameters.*/
int SystemBPSample = 16;
int MaxChannels = 2;
long int Rate = 22050;


/*  where the data gets munged to.*/
short int CombiningBuffer[MAXBUFSIZE];
void initializeCombiningBuffer();

void playWavFragment() {
	/* SNDFILE *wavfile, int source) {*/
	audio_buf_info leftover;
	int mydata;			/*  DSP buffer size... used to keep data flowing*/

	/*  Only write if there is the need to write data. - dont want*/
	/*  to buffer too much; want sound to be responsive but smooth*/
	/*  -- this call tells us how much room there is.*/


	/*  Find out how much data was processed by the sound card since the*/
	/*  last write. First time through we assume that the sound card*/
	/*  buffer is flushed, and that we have to write data.*/

	/* printf ("start of playWavFragment source %d\n",source);*/
	/* */

	/*  is the dspFile open? Maybe it failed on open??*/
	if (dspFile == -1) {
		/* printf ("dsp not open\n");*/
		return;
	}

	if (DSPplaying != 0) {
		/*  first time through*/
		/* printf ("first time through\n");*/
		DSPplaying =  0;
		mydata = 0;
		readSize = 0;
		bytesPerCycle = MAXBUFSIZE/2; /*  make an assumption.*/
	} else {
		/*  we have done this before since the file open...*/
		mydata = soundcardBufferEmptySize - soundcardBufferCurrentSize;
		/* printf ("SCES %d SCCS %d mydata %d bytes_remaining %ld\n",*/
		/* 		soundcardBufferEmptySize,*/
		/* 		soundcardBufferCurrentSize,mydata,wavfile->bytes_remaining);*/


		/*  lets try some scaling here.*/
		/*  did we (or are we close to) running out of data?*/
		/*if ((mydata <= 0x4ff) && (bytesPerCycle < BUFSIZE*16)) {
			//printf ("increasing bps\n");
			bytesPerCycle += 0x100;
		}*/
	}
	/* printf ("md %d, bps %d rate %f bytes/sec\n",mydata, bytesPerCycle, fps*bytesPerCycle);*/

	/*  Should we read and write?*/
	if (mydata <= (bytesPerCycle*2)) {
		initializeCombiningBuffer();
		/* printf ("icb, smd %d\n",bytesPerCycle);*/
		streamMoreData(bytesPerCycle);
	}

	if (ioctl(dspFile, SNDCTL_DSP_GETOSPACE,&leftover) <0) {
		printf ("error, SNDCTL_DSP_GETOSPACE\n");
		dspFile = -1;
	}
	/* printf ("space leftover is %d\n",leftover.bytes);*/
	soundcardBufferCurrentSize = leftover.bytes;
}



/*  WAV file header read in, lets get the rest of the data ready.*/
SNDFILE *initiateWAVSound (SNDFILE *wavfile,int mynumber) {
	wavfile->type=WAVFILE;
	return wavfile;
}

/*  Close the DSP, release memory.*/
void closeDSP () {
	if (dspBlock!=NULL) free(dspBlock);
	if (dspFile>=0) close(dspFile);
	dspFile = -1;
	DSPplaying = -1;
}

void initiateDSP() {
	int i;
	audio_buf_info leftover;

	if ( (dspFile = open("/dev/dsp",O_WRONLY|O_NONBLOCK))
	/* if ( (dspFile = open("/dev/dsp",O_NDELAY))*/
                                   == -1 ) {
		printf ("FreeWRL::SoundEngine::open /dev/dsp problem (is something else using it?)\n");
		dspFile=-1;
		return;
	}

	i = (N_FRAGMENTS<<16) | FRAG_SIZE;
	if ( ioctl(dspFile, SNDCTL_DSP_SETFRAGMENT,
                             &i) == -1 ) {
		printf("ioctl set fragment problem\n");
		dspFile=-1;
		return ;
	}

	/*  quick calculation to find out how much space the DRIVER thinks*/
	/*  we have when the sound card buffer is empty.*/
	if (ioctl(dspFile, SNDCTL_DSP_GETOSPACE,&leftover) <0) {
		printf ("error, SNDCTL_DSP_GETOSPACE\n");
		dspFile = -1;
	}
	soundcardBufferEmptySize = leftover.bytes;
	/* printf ("can write a possible amount of %d bytes\n",leftover.bytes);*/

	/*  set for 16 bit samples.*/
	if (ioctl(dspFile,SNDCTL_DSP_SETFMT,&SystemBPSample)<0) {
		printf ("unable to set DSP bit size to %d\n",SystemBPSample);
		dspFile = -1; /*  flag an error*/
	}

	/*  set for stereo.*/
	if (ioctl(dspFile,SNDCTL_DSP_STEREO,&MaxChannels)<0) {
		printf ("unable to set mono/stereo mode to %d\n",MaxChannels);
		dspFile = -1; /*  flag an error*/
	}

	/*  Set rate.*/
	if (ioctl(dspFile,SNDCTL_DSP_SPEED,&Rate)<0) {
		printf ("unable to set DSP sampling rate to %ld\n",Rate);
		dspFile = -1; /*  flag an error*/
	}


    return ;
}


void initializeCombiningBuffer() {
	int count;

	for (count =0; count < MAXBUFSIZE; count++) {
		/* CombiningBuffer[count] = 32767;*/
		CombiningBuffer[count] = 0;
		/* printf ("initialized %d\n",CombiningBuffer[count]);*/
	}
}

/*  add this new stream to our standard buffer. Convert as appropriate.*/
void addToCombiningBuffer(int source,int readSize, int offset) {
	int tc;
	short int *siptr;
	int tmp;
	int ampl;
	int lbal, rbal;


	/* printf ("afer start, offset = %d readSize %d \n", offset,readSize);*/

	ampl = 100 - (sndfile[source]->ampl);
	if (ampl < 1) ampl = 1;	/*  stops divide by zero errors*/

	switch (sndfile[source]->FormatChunk.wBitsPerSample) {
		case SIXTEEN: {
			siptr = (short int *)(sndfile[source]->data);
			lbal = sndfile[source]->balance;
			rbal = 100 - lbal;

			/*  lets try this... basically, either add to or*/
			/*  reduce balance, but keep average at 1.0*/
			rbal = rbal*2;
			lbal = lbal*2;

			/* printf ("lbal %d rbal %d\n",lbal, rbal);*/

			for (tc=0; tc<(readSize/2); tc++) {
				/*  get value, and adjust for volume*/
				if ((tc & 0x01) == 0) {
					/* printf ("left\n");*/
					tmp = *siptr;
					tmp = (tmp *100)/ampl;
					tmp = (tmp * lbal) / 50;
					tmp = tmp/100;
					/*  balance*/
				} else {
					/* printf ("right\n");*/
					tmp = *siptr;
					tmp = (tmp *100)/ampl;
					tmp = (tmp * rbal) / 50;
					tmp = tmp/100;
				}

				/*  combine them, then check for overflow*/
				tmp = tmp + (int)CombiningBuffer[offset];
				if ((tmp > 32767) || (tmp <-32768)) {
					/* printf ("have a problem, %d\n",tmp);*/
					if (tmp > 32767) {
						tmp = 32767;
					} else {
						tmp = -32768;
					}
				}

				/* CombiningBuffer[offset] += tmp;*/
				CombiningBuffer[offset] = tmp;
				offset++;
				siptr++;
			}
			break;
		}

		default: {
			 printf ("woops, addToStreaming not for this type\n");
		}
	}
}
