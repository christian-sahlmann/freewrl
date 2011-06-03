/*
=INSERT_TEMPLATE_HERE=

$Id: ColladaParser.h,v 1.6 2011/06/03 16:08:14 davejoubert Exp $

Collada parser functions.

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


#ifndef __FREEWRL_COLLADA_PARSER_H__
#define __FREEWRL_COLLADA_PARSER_H__

int freewrl_XML_GetCurrentLineNumber();

#define PARENTSTACKSIZE 256
#define LINE freewrl_XML_GetCurrentLineNumber()
/* this ifdef sequence is kept around, for a possible Microsoft Vista port */
#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif


extern int CDATA_Text_curlen;
extern char *CDATA_Text;

//extern struct X3D_Node *colladaParentStack[PARENTSTACKSIZE];

/* See: .... = NULL ; make sure we know the state of the new Top of Stack */
/*
#define INCREMENT_PARENTINDEXC \
        if (parentIndex < (PARENTSTACKSIZE-2))  { \
                parentIndex++; \
                colladaParentStack[parentIndex] = NULL; \
        } else ConsoleMessage ("ColladaParser, line %d stack overflow",LINE);
*/

int ColladaParse (struct X3D_Group* myParent, const char *inputstring);

#endif /*  __FREEWRL_COLLADA_PARSER_H__ */
