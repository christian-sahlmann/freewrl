
typedef union un1 {
	int i;
	float f;
	void *p;
} rbItem;

typedef struct rb1 {
	int head ;
	int tail ;
	int noOfElements ;
	void * data;
} RingBuffer ;

#define FORCE_GUARD_ELEMENT 1
#define TRACK_RINGBUFFER_MSG 0

/* Con/De-structors */
RingBuffer * NewRingBuffer (int elCount) ;
/*
 * You should only use makeEmpty for floats and ints.
 *
 * For pointers you should first do something like this:
 *
	while (!RingBuffer_testEmpty(buffer) ;
		toClean = RingBuffer_pullUnion(buffer).p ;
		free(toClean);
	}
 */
void RingBuffer_makeEmpty(RingBuffer * buffer) ;
void RingBuffer_freeDataArea(RingBuffer * buffer) ;


/* helpers */
int RingBuffer_qLen(RingBuffer * buffer) ;
int RingBuffer_freeLen(RingBuffer * buffer) ;
int RingBuffer_testEmpty(RingBuffer * buffer) ;
int RingBuffer_testFull(RingBuffer * buffer) ;

/* Add data to back of queue */
int RingBuffer_pushInt(RingBuffer * buffer, int newInt) ;
int RingBuffer_pushFloat(RingBuffer * buffer, float newFloat) ;
int RingBuffer_pushPointer(RingBuffer * buffer, void *newPointer) ;


/* remove data from the front of the queue */
/* Note: RingBuffer_pullUnion is not bulletproof!! You should
ALWAYS first test RingBuffer_testEmpty BEFORE a RingBuffer_pullUnion.

However, because it is a pull rather than a push then not checking
is not fatal.
*/

rbItem * RingBuffer_pullUnion(RingBuffer * buffer) ;
rbItem * RingBuffer_peekUnion(RingBuffer * buffer) ;

