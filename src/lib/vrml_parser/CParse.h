/*
=INSERT_TEMPLATE_HERE=

$Id: CParse.h,v 1.9 2011/06/09 17:15:14 dug9 Exp $

VRML-parsing routines in C.

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


#ifndef __FREEWRL_CPARSE_H__
#define __FREEWRL_CPARSE_H__


/* for scanning and determining whether a character is part of a valid X3D name */
#define IS_ID_REST(c) \
 (c>0x20 && c!=0x22 && c!=0x23 && c!=0x27 && c!=0x2C && c!=0x2E && c!=0x3a && c!=0x5B && \
  c!=0x5C && c!=0x5D && c!=0x7B && c!=0x7D && c!=0x7F)
#define IS_ID_FIRST(c) \
 (IS_ID_REST(c) && (c<0x30 || c>0x39) && c!=0x2B && c!=0x2D)

BOOL cParse(void*, unsigned, const char*);

/* Destroy all data associated with the currently parsed world kept. */
#define destroyCParserData(me) \
 parser_destroyData(me)

/* Some accessor-methods */
struct X3D_Node* parser_getNodeFromName(const char*);
char* parser_getNameFromNode(struct X3D_Node*);
char* parser_getPROTONameFromNode(struct X3D_Node*);
//extern struct VRMLParser* globalParser;

/* tie assert in here to give better failure methodology */
/* #define ASSERT(cond) if(!(cond)){fw_assert(__FILE__,__LINE__);} */
/* void fw_assert(char *,int); */


#endif /* __FREEWRL_CPARSE_H__ */
