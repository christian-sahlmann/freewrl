/*
=INSERT_TEMPLATE_HERE=

$Id: Snapshot.h,v 1.7 2009/10/05 15:07:23 crc_canada Exp $

Screen snapshot.

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
#ifdef WIN32   
/* win32 has abort() in stdlib.h - is that what we want? */
#else
extern void abort();
#endif


#endif /* __FREEWRL_SNAPSHOT_H__ */
