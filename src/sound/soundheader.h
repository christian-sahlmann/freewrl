/*********************************************************************
 *
 * FreeWRL SoundServer engine
 *
 *  Copyright (C) 2002 John Stewart, CRC Canada.
 *  DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
 *  See the GNU Library General Public License (file COPYING in the distribution)
 *  for conditions of use and redistribution.
 *
 *********************************************************************/

#ifndef __FREEWRL_SND_SOUND_H__
#define __FREEWRL_SND_SOUND_H__


#define  UNKNOWNFILE 0
#define  WAVFILE 3
#define	 MPGFILE 1
#define  MP3FILE 2

#define DSP "/dev/dsp"

#define MAXSOURCES 128
#define BUFSIZE  512
#define MAXBUFSIZE 8192
#define NUMBUFS 2

/* number of fragments */
#define N_FRAGMENTS 	48
/* a buffersize of 2^8 = 256 bytes */
#define FRAG_SIZE 	8

#define FormatID 'fmt '   /* chunkID for Format Chunk. NOTE: There is a space at the end of this ID. */

typedef struct {
	char           chunkID[4];
	int            chunkSize;
	short          wFormatTag;
	unsigned short wChannels;
	unsigned int  dwSamplesPerSec;
	unsigned int  dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
} fmtChnk;


#define DataID 'data'  /* chunk ID for data Chunk */

typedef struct {
	char	chunkID[4];
	int32_t chunkSize;
} datChnk;

typedef struct {
	long     mtype;  /* message type */
	char    msg[256]; /* message data */
} FWSNDMSG;

typedef struct {
	int	type;		/* // 1 = wav, etc, etc */
	FILE	*fd;		/* // file descriptor of sound file */
	char	data[MAXBUFSIZE];	/* // data read in from file */
	int	dataptr;	/* // where in the data field we are */
	int	wavdataoffset;	/* // this is the offset from start of file to wave data */
	float	pitch;		/* // pitch multiplier */
	int bytes_remaining;	/* // how many bytes until EOF is reached - used in */
	/* // playing the file */
	/* // */
	int ampl;               /* // Gain of this sound */
	int balance;		/* // L-R pairing. */

	fmtChnk	FormatChunk;	/* // the "fmt " chunk is copied here */
	datChnk DataChunk;	/* // the one and only one "data" chunk is copied here */

} SNDFILE;

extern short int CombiningBuffer[MAXBUFSIZE];
extern int MaxAvgBytes;
extern int active[MAXSOURCES];
extern int registered[MAXSOURCES];
void addToCombiningBuffer(int count, int readSize, int offset);
extern SNDFILE *sndfile[MAXSOURCES];
extern int loop[MAXSOURCES];
extern int current_max;
extern int readSize;

extern int dspFile;
extern float fps;
void closeDSP();
SNDFILE *initiateWAVSound (SNDFILE *wavfile,int mynum);
void playWavFragment ();
void initiateDSP ();
void rewind_to_beginning (SNDFILE *wavfile);
void recordMaxWavParams(SNDFILE *me);
void streamMoreData(int size);
void adjustAmplitude(int source, int readSize);

/* // bits per sample: */
#define	EIGHT 		8
#define SIXTEEN		16
#define	THIRTYTWO	32

#define UNINITWAV	-20000


#endif /* __FREEWRL_SND_SOUND_H__ */
