extern int snapCount;
extern int maxSnapImages;          /* --maximg command line parameter              */
extern int snapGif;            /* --gif save as an animated GIF, not mpg       */
extern char *snapseqB;          /* --seqb - snap sequence base filename         */
extern char *snapsnapB;         /* --snapb -single snapshot files               */
extern char *seqtmp;            /* --seqtmp - directory for temp files          */
extern int snapsequence;	/* --seq - snapshot sequence, not single click	*/
extern int doSnapshot;		/* are we doing a snapshot?			*/
void setSnapshot();		/* set a snapshot going				*/
void Snapshot();
extern void abort();
