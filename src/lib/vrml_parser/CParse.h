/*
=INSERT_TEMPLATE_HERE=

$Id: CParse.h,v 1.2 2008/11/27 00:27:18 couannette Exp $

VRML-parsing routines in C.

*/

#ifndef __FREEX3D_CPARSE_H__
#define __FREEX3D_CPARSE_H__


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
extern struct VRMLParser* globalParser;

/* tie assert in here to give better failure methodology */
#define ASSERT(cond) if(!(cond)){fw_assert(__FILE__,__LINE__);}
void fw_assert(char *,int);


#endif /* __FREEX3D_CPARSE_H__ */
