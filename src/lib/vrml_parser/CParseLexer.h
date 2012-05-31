/*
=INSERT_TEMPLATE_HERE=

$Id: CParseLexer.h,v 1.13 2012/05/31 19:06:42 crc_canada Exp $

Lexer (input of terminal symbols) for CParse

Tables of user-defined IDs: userNodeNames (DEFs) is scoped with a
simple stack, as every PROTO has its scope completely *different* from
the rest of the world.  userNodeTypes (PROTO definitions) needs to be
available up through the whole stack, values are stored in a vector,
and the indices where each stack level ends are stored in a stack.
fields are not scoped and therefore stored in a simple vector.

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


#ifndef __FREEWRL_CPARSE_LEXER_H__
#define __FREEWRL_CPARSE_LEXER_H__

/* for Stack typedef */
#include "../scenegraph/Vector.h"

/* Undefined ID (for special "class", like builtIn and exposed) */
#ifdef ID_UNDEFINED
#undef ID_UNDEFINED
#endif
#define ID_UNDEFINED	-1

#define LEXER_INPUT_STACK_MAX 16

/* This is our lexer-object. */
struct VRMLLexer
{
 char* nextIn;	/* Next input character. */
 char* startOfStringPtr[LEXER_INPUT_STACK_MAX]; /* beginning address of string, for FREE calls */
 char* curID;	/* Currently input but not lexed id. */
 BOOL isEof;	/* Error because of EOF? */

 int lexerInputLevel; /* which level are we at? used for putting PROTO expansions into the input stream, etc */
 char *oldNextIn[LEXER_INPUT_STACK_MAX];     /* old nextIn pointer, before the push */

 Stack* userNodeNames;
 struct Vector* userNodeTypesVec;
 Stack* userNodeTypesStack;
 struct Vector* user_initializeOnly;
 struct Vector* user_inputOutput;
 struct Vector* user_inputOnly;
 struct Vector* user_outputOnly;
};

/* Constructor and destructor */
struct VRMLLexer* newLexer();
void deleteLexer(struct VRMLLexer*);

/* Other clean up. */
void lexer_destroyData(struct VRMLLexer* me);
void lexer_destroyIdStack(Stack*);

/* Count of elements to pop off the PROTO vector for scope-out */
#define lexer_getProtoPopCnt(me) \
 (vectorSize(me->userNodeTypesVec)-stack_top(int, me->userNodeTypesStack))

/* Set input */
void lexer_fromString (struct VRMLLexer *, char *);
void lexer_forceStringCleanup (struct VRMLLexer *me);

/* Is EOF? */
#define lexer_eof(me) \
 ((me)->isEof && !(me)->curID)

/* int index -> char* conversion */
#define lexer_stringUFieldName(me, index, type) \
 vector_get(char*, me->user_##type, index)
#define lexer_stringUser_initializeOnly(me, index) \
 lexer_stringUFieldName(me, index, initializeOnly)
#define lexer_stringUser_inputOutput(me, index) \
 lexer_stringUFieldName(me, index, inputOutput)
#define lexer_stringUser_inputOnly(me, index) \
 lexer_stringUFieldName(me, index, inputOnly)
#define lexer_stringUser_outputOnly(me, index) \
 lexer_stringUFieldName(me, index, outputOnly)
/* User field name -> char*, takes care of access mode */
const char* lexer_stringUser_fieldName(struct VRMLLexer* me, int name, int mode);

/* Skip whitespace and comments. */
void lexer_skip(struct VRMLLexer*);

/* Ensures that curID is set. */
BOOL lexer_setCurID(struct VRMLLexer*);

/* Some operations with IDs */
void lexer_scopeIn(struct VRMLLexer*);
void lexer_scopeOut(struct VRMLLexer*);
void lexer_scopeOut_PROTO(struct VRMLLexer*);
BOOL lexer_keyword(struct VRMLLexer*, int);
BOOL lexer_specialID(struct VRMLLexer*, int* retB, int* retU,
 const char**, const int, struct Vector*);
BOOL lexer_specialID_string(struct VRMLLexer*, int* retB, int* retU,
 const char**, const int, struct Vector*, const char*);
BOOL lexer_defineID(struct VRMLLexer*, int*, struct Vector*, BOOL);
#define lexer_defineNodeName(me, ret) \
 lexer_defineID(me, ret, stack_top(struct Vector*, me->userNodeNames), TRUE)
#define lexer_defineNodeType(me, ret) \
 lexer_defineID(me, ret, me->userNodeTypesVec, FALSE)
#define lexer_define_initializeOnly(me, ret) \
 lexer_defineID(me, ret, me->user_initializeOnly, TRUE)
#define lexer_define_inputOutput(me, ret) \
 lexer_defineID(me, ret, me->user_inputOutput, TRUE)
#define lexer_define_inputOnly(me, ret) \
 lexer_defineID(me, ret, me->user_inputOnly, TRUE)
#define lexer_define_outputOnly(me, ret) \
 lexer_defineID(me, ret, me->user_outputOnly, TRUE)
BOOL lexer_initializeOnly(struct VRMLLexer*, int*, int*, int*, int*);
BOOL lexer_event(struct VRMLLexer*, struct X3D_Node*,
 int*, int*, int*, int*, int routeToFrom);
#define lexer_inputOnly(me, node, a, b, c, d) \
 lexer_event(me, node, a, b, c, d, ROUTED_FIELD_EVENT_IN)
#define lexer_outputOnly(me, node, a, b, c, d) \
 lexer_event(me, node, a, b, c, d, ROUTED_FIELD_EVENT_OUT)
#define lexer_node(me, r1, r2) \
 lexer_specialID(me, r1, r2, NODES, NODES_COUNT, me->userNodeTypesVec)
#define lexer_nodeName(me, ret) \
 lexer_specialID(me, NULL, ret, NULL, 0, \
  stack_top(struct Vector*, me->userNodeNames))
#define lexer_protoFieldMode(me, r) \
 lexer_specialID(me, r, NULL, PROTOKEYWORDS, PROTOKEYWORDS_COUNT, NULL)
#define lexer_fieldType(me, r) \
 lexer_specialID(me, r, NULL, FIELDTYPES, FIELDTYPES_COUNT, NULL)
int lexer_string2id(const char*, const struct Vector*);
#define lexer_nodeName2id(me, str) \
 lexer_string2id(str, stack_top(struct Vector*, me->userNodeNames))

/* Input the basic literals */
BOOL lexer_int32(struct VRMLLexer*, vrmlInt32T*);
BOOL lexer_float(struct VRMLLexer*, vrmlFloatT*);
BOOL lexer_double(struct VRMLLexer*, vrmlDoubleT*);
BOOL lexer_string(struct VRMLLexer*, vrmlStringT*);

/* Checks for the five operators of VRML */
BOOL lexer_operator(struct VRMLLexer*, char);
#define lexer_point(me) \
 lexer_operator(me, '.')
#define lexer_openCurly(me) \
 lexer_operator(me, '{')
#define lexer_closeCurly(me) \
 lexer_operator(me, '}')
#define lexer_openSquare(me) \
 lexer_operator(me, '[')
#define lexer_closeSquare(me) \
 lexer_operator(me, ']')
#define lexer_colon(me) \
 lexer_operator(me,':')

/* recursively skip to the closing curly bracket */
void skipToEndOfOpenCurly(struct VRMLLexer *me, int level);

void concatAndGiveToLexer(struct VRMLLexer *me, const char *str_a, const char *str_b);

/* other function protos */
BOOL lexer_field(struct VRMLLexer* me,int* retBO, int* retBE, int* retUO, int* retUE);


#endif /* __FREEWRL_CPARSE_LEXER_H__ */
