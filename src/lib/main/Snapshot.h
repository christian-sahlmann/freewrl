/*
=INSERT_TEMPLATE_HERE=

$Id: Snapshot.h,v 1.4 2009/02/11 15:12:54 istakenv Exp $

Screen snapshot.

*/

#ifndef __FREEWRL_SNAPSHOT_H__
#define __FREEWRL_SNAPSHOT_H__


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


#endif /* __FREEWRL_SNAPSHOT_H__ */
