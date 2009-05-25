/*
=INSERT_TEMPLATE_HERE=

$Id: X3DProtoScript.c,v 1.16 2009/05/25 16:54:20 crc_canada Exp $

???

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>


#include "../vrml_parser/Structs.h"
#include "../main/headers.h"
#include "../vrml_parser/CParseGeneral.h"
#include "../scenegraph/Vector.h"
#include "../vrml_parser/CFieldDecls.h"
#include "../world_script/CScripts.h"
#include "../world_script/fieldSet.h"
#include "../vrml_parser/CParseParser.h"
#include "../vrml_parser/CParseLexer.h"
#include "../vrml_parser/CProto.h"
#include "../vrml_parser/CParse.h"

#include "X3DParser.h"
#include "X3DProtoScript.h"


static int currentProtoDeclare  = INT_ID_UNDEFINED;
static int MAXProtos = 0;
static int curProDecStackInd = 0;
static int currentProtoInstance = INT_ID_UNDEFINED;

/* for parsing fields in PROTO expansions */
/* FIELD_END is an ascii string that will pass the XML parser, but should not be found in a field value */
#define FIELD_END "\t \t\n" 
#define strIS "<IS>"
#define strNOTIS "</IS>"
#define strCONNECT  "<connect"
#define strNODEFIELD "nodeField"
#define strPROTOFIELD "protoField"
#define MAX_ID_SIZE 1000
#define NODEFIELD_EQUALS "nodeField=\""
#define PROTOFIELD_EQUALS "protoField=\""

/* for parsing script initial fields */
#define MPFIELDS 4
#define MP_NAME 0
#define MP_ACCESSTYPE 1
#define MP_TYPE 2
#define MP_VALUE 3

/* ProtoInstance table This table is a dynamic table that is used for keeping track of ProtoInstance field values... */
static int curProtoInsStackInd = -1;

struct PROTOInstanceEntry {
	char *name[PROTOINSTANCE_MAX_PARAMS];
	char *value[PROTOINSTANCE_MAX_PARAMS];
	char *defName;
	int container;
	int paircount;
};
static struct PROTOInstanceEntry ProtoInstanceTable[PROTOINSTANCE_MAX_LEVELS];

/* PROTO table */
struct PROTOnameStruct {
	char *name;
	FILE *fileDescriptor;
	char *fileName;
	int charLen;
	int fileOpen;
};
static struct PROTOnameStruct *PROTONames = NULL;


/* Script table - script parameter names, values, etc. */
struct ScriptFieldStruct {
	int scriptNumber;
	int fromScriptNotPROTO;
	struct Uni_String *fieldName;
	struct Uni_String *value;
	int type;
	int kind;
	int offs;
};

static struct ScriptFieldStruct *ScriptFieldNames = NULL;
static int ScriptFieldTableSize = INT_ID_UNDEFINED;
static int MAXScriptFieldParams = 0;


/****************************** PROTOS ***************************************************/

/* we are closing off a parse of an XML file. Lets go through and free/unlink/cleanup. */
void freeProtoMemory () {
	int i;

	
	#ifdef X3DPARSERVERBOSE
	printf ("freeProtoMemory, currentProtoDeclare is %d PROTONames = %d \n",currentProtoDeclare,PROTONames);
	#endif

	if (PROTONames != NULL) {
		for (i=0; i<= currentProtoDeclare; i++) {
			if (PROTONames[i].fileOpen) fclose(PROTONames[i].fileDescriptor); /* should never happen... */

			FREE_IF_NZ (PROTONames[i].name);
			if (PROTONames[i].fileName != NULL) UNLINK (PROTONames[i].fileName);
			free (PROTONames[i].fileName); /* can not FREE_IF_NZ this one as it's memory is not kept track of by MALLOC */

		}
		FREE_IF_NZ(PROTONames);
	}

	currentProtoDeclare  = INT_ID_UNDEFINED;
	MAXProtos = 0;

	#ifdef X3DPARSERVERBOSE
	printf ("freeProtoMemory,ScriptFieldNames is %d ScriptFieldTableSize %d, MAXScriptFieldParams %d\n",ScriptFieldNames, ScriptFieldTableSize, MAXScriptFieldParams);
	#endif

	if (ScriptFieldNames != NULL) {
		for (i=0; i<=ScriptFieldTableSize; i++) {
			if (ScriptFieldNames[i].fieldName != NULL) {
				FREE_IF_NZ(ScriptFieldNames[i].fieldName->strptr);
				FREE_IF_NZ(ScriptFieldNames[i].fieldName);
			}
			if (ScriptFieldNames[i].value != NULL) {
				FREE_IF_NZ(ScriptFieldNames[i].value->strptr);
				FREE_IF_NZ(ScriptFieldNames[i].value);
			}
		}
		FREE_IF_NZ(ScriptFieldNames);
	}
	ScriptFieldTableSize = INT_ID_UNDEFINED;
	MAXScriptFieldParams = 0;
}


/* record each field of each script - the type, kind, name, and associated script */
static void registerProto(const char *name) {
	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("registerProto for %s\n",name);
	#endif


	/* ok, we got a name and a type */
	if (currentProtoDeclare >= MAXProtos) {
		/* oooh! not enough room at the table */
		MAXProtos += 10; /* arbitrary number */
		PROTONames = (struct PROTOnameStruct*)REALLOC (PROTONames, sizeof(*PROTONames) * MAXProtos);
	}

	PROTONames[currentProtoDeclare].name = STRDUP((char *)name);
	PROTONames[currentProtoDeclare].fileName = tempnam("/tmp","freewrl_proto");
	PROTONames[currentProtoDeclare].fileDescriptor = fopen(PROTONames[currentProtoDeclare].fileName,"w");
#ifndef TRY
	PROTONames[currentProtoDeclare].charLen =  fprintf (PROTONames[currentProtoDeclare].fileDescriptor,
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Scene>\n");
#else
	PROTONames[currentProtoDeclare].charLen = 0;
#endif
	PROTONames[currentProtoDeclare].fileOpen = TRUE;

	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("opened name %s for PROTO name %s id %d\n",PROTONames[currentProtoDeclare].fileName,
		PROTONames[currentProtoDeclare].name,currentProtoDeclare);
	#endif
}

void parseProtoInstanceFields(const char *name, const char **atts) {
	int count;
	int picatindex = 0;
	int picatmalloc = 0;

	#define INDEX ProtoInstanceTable[curProtoInsStackInd].paircount	
	#define ZERO_NAME_VALUE_PAIR \
		ProtoInstanceTable[curProtoInsStackInd].name[INDEX] = NULL; \
		ProtoInstanceTable[curProtoInsStackInd].value[INDEX] = NULL;

	#define VERIFY_PCAT_LEN(myLen) \
		if ((myLen + 10) >= picatmalloc) { \
			if (picatmalloc == 0) { \
				picatmalloc = 1024; \
				picatindex = 0; \
			} \
			while ((picatmalloc + 20) < myLen) picatmalloc *= 2; \
			ProtoInstanceTable[curProtoInsStackInd].value[INDEX] = REALLOC(ProtoInstanceTable[curProtoInsStackInd].value[INDEX], picatmalloc); \
		} \

	#define PICAT_CAT(myStr,myLen) \
		memcpy(&ProtoInstanceTable[curProtoInsStackInd].value[INDEX][picatindex], myStr,myLen); \
		picatindex += myLen; \
		ProtoInstanceTable[curProtoInsStackInd].value[INDEX][picatindex+1] = '\0'; 
		
	#define PICAT(myStr,myLen) \
		VERIFY_PCAT_LEN(myLen) \
		PICAT_CAT(myStr,myLen)


	#ifdef X3DPARSERVERBOSE
	printf ("parsing PRotoInstanceFields for %s at level %d\n",name,curProtoInsStackInd);
	#endif

	/* this should be a <fieldValue...> tag here */
	/* eg: <fieldValue name='imageName' value="helpers/brick.png"/> */
	if (strcmp(name,"fieldValue") == 0) {
		ZERO_NAME_VALUE_PAIR
		for (count = 0; atts[count]; count += 2) {
			#ifdef X3DPARSERVERBOSE
			printf ("ProtoInstanceFields: %s=\"%s\"\n",atts[count], atts[count+1]);
			#endif

			/* add this to our instance tables */
			/* is this the name field? */
			if (strcmp("name",atts[count])==0) 
				ProtoInstanceTable[curProtoInsStackInd].name[INDEX] = STRDUP(atts[count+1]);
			if (strcmp("value",atts[count])==0) 
				ProtoInstanceTable[curProtoInsStackInd].value[INDEX] = STRDUP(atts[count+1]);

			/* did we get both a name and a value? */
			if ((ProtoInstanceTable[curProtoInsStackInd].name[INDEX] != NULL) &&
			    (ProtoInstanceTable[curProtoInsStackInd].value[INDEX] != NULL)) {
				INDEX++;
				ZERO_NAME_VALUE_PAIR
			}

			if (INDEX>=PROTOINSTANCE_MAX_PARAMS) {
				ConsoleMessage ("too many parameters for ProtoInstance, sorry...\n");
				INDEX=0;
			}
		}
	} else if (strcmp(name,"ProtoInstance") != 0) {
		/* maybe this is a SFNode value, as in: 
                    <fieldValue name='relay'> <Script USE='CameraRelay'/> </fieldValue>

		  if this IS the case, we will be down here where the atts parameter will be the "USE=CameraRelay" pair
		*/
		
		/* printf ("ProtoInstance, looking for fieldValue, found string:%s: current name=%s value=%s, index %d\n",name,
			ProtoInstanceTable[curProtoInsStackInd].name[INDEX], ProtoInstanceTable[curProtoInsStackInd].value[INDEX],INDEX); */

		/* allow ONLY USEs here, at least for now? */
		if (atts != NULL) {
			if (strcmp("USE",atts[0]) == 0) {
				struct X3D_Node *rv;
				char *val = MALLOC(20);

				rv = DEFNameIndex(atts[1],NULL,FALSE);
				/* X3D_Node *DEFNameIndex (const char *name, struct X3D_Node* node, int force) */
				/* printf ("found USE in proto expansion for SFNode, is %u\n",rv); */
				sprintf (val, "%u",rv);
				ProtoInstanceTable[curProtoInsStackInd].value[INDEX] = val;
			} else {
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 
				/* NOT SURE THE FOLLOWING IS A GOOD IDEA */ 

				/* just append this, and hope for the best */
				for (count = 0; atts[count]; count += 2) {
					PICAT(atts[count],strlen(atts[count]))
					PICAT("=",1)
					PICAT(atts[count+1],strlen(atts[count+1]))
				}
			}
		}
		/* printf ("NOW ProtoInstance, looking for fieldValue, found string:%s: current name=%s value=%s, index %d\n",name,
			ProtoInstanceTable[curProtoInsStackInd].name[INDEX], ProtoInstanceTable[curProtoInsStackInd].value[INDEX],INDEX); */
		INDEX++;
		ZERO_NAME_VALUE_PAIR
	}
	#undef INDEX
}


/***********************************************************************************/


void dumpProtoBody (const char *name, const char **atts) {
	int count;

	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("dumping ProtoBody for %s\n",name);
	#endif

	if (PROTONames[currentProtoDeclare].fileOpen) {
		PROTONames[currentProtoDeclare].charLen += fprintf (PROTONames[currentProtoDeclare].fileDescriptor, "<%s",name);
		for (count = 0; atts[count]; count += 2) {
			/* printf ("dumpProtoBody - do we need to worry about quotes in :%s: \n",atts[count+1]); */
			if (atts[count+1][0] == '"') {
			    /* put single quotes around this one */
			    PROTONames[currentProtoDeclare].charLen += 
				fprintf (PROTONames[currentProtoDeclare].fileDescriptor," %s='%s' %s",atts[count],atts[count+1],FIELD_END);
			} else {
			    PROTONames[currentProtoDeclare].charLen += 
				fprintf (PROTONames[currentProtoDeclare].fileDescriptor," %s=\"%s\" %s",atts[count],atts[count+1],FIELD_END);
			}
		}
		PROTONames[currentProtoDeclare].charLen += fprintf (PROTONames[currentProtoDeclare].fileDescriptor,">\n");
	}
}

void dumpCDATAtoProtoBody (char *str) {
	if (PROTONames[currentProtoDeclare].fileOpen) {
		PROTONames[currentProtoDeclare].charLen += 
			fprintf (PROTONames[currentProtoDeclare].fileDescriptor,"<![CDATA[%s]]>",str);
	}
}

void endDumpProtoBody (const char *name) {
	/* we are at the end of the ProtoBody, close our tempory file */
	if (PROTONames[currentProtoDeclare].fileOpen) {
		#ifndef try
		PROTONames[currentProtoDeclare].charLen += fprintf (PROTONames[currentProtoDeclare].fileDescriptor, "\n</Scene>\n");
		#endif

		fclose (PROTONames[currentProtoDeclare].fileDescriptor);
		PROTONames[currentProtoDeclare].fileOpen = FALSE;

		#ifdef X3DPARSERVERBOSE
			TTY_SPACE
			printf ("endDumpProtoBody, just closed %s, it is %d characters long\n",
				PROTONames[currentProtoDeclare].fileName, PROTONames[currentProtoDeclare].charLen);
		#endif
	}
}


/* find a value for the proto field on invocation. First look at the ProtoInstance, if not there, then look
   at the ProtoDeclare for the field. */
static char *getProtoValue(int ProtoInvoc, char *id) {
	char *retptr;
	int i;

	#ifdef X3DPARSERVERBOSE
	printf ("getProtoValue for proto %d, char :%s:\n",ProtoInvoc, id);
	#endif

	/* get the start/end value pairs, and copy them into the id field. */
printf ("getProtoValue, curProtoInsStackInd %d, MAX %d\n",curProtoInsStackInd, PROTOINSTANCE_MAX_LEVELS);
	if ((curProtoInsStackInd < 0) || (curProtoInsStackInd >= PROTOINSTANCE_MAX_LEVELS)) {
		return "";
	} else {
		/* is this to be matched in the ProtoInstance fields? */

		for (i=0; i<ProtoInstanceTable[curProtoInsStackInd].paircount; i++) {
			#ifdef X3DPARSERVERBOSE
				printf (" 	getProtoValue - ProtoInstance - comparing ind %d, :%s:  :%s:\n",
					i,id,ProtoInstanceTable[curProtoInsStackInd].name[i]);
			#endif
			if (strcmp(id,ProtoInstanceTable[curProtoInsStackInd].name[i]) == 0) {
				/* printf ("getProtoValue, found name!\n"); */
				return ProtoInstanceTable[curProtoInsStackInd].value[i];
			}
		}

		/* no, maybe the field is in the ProtoInterface definitions? */
		#ifdef X3DPARSERVERBOSE
		printf ("have to look for id :%s: in ProtoInterface\n",id);
		#endif

		if (getFieldValueFromProtoInterface (id, ProtoInvoc, &retptr)) {
			return retptr;
		}
	}

	/* if we are here, we did not find that parameter */
	ConsoleMessage ("ProtoInstance <%s>, could not find parameter <%s>",
			PROTONames[ProtoInvoc].name, id);

	return "";
}

/* handle a <ProtoInstance> tag */
void parseProtoInstance (const char **atts) {
	int count;
	int nameIndex = INT_ID_UNDEFINED;
	int containerIndex = INT_ID_UNDEFINED;
	int containerField = INT_ID_UNDEFINED;
	int defNameIndex = INT_ID_UNDEFINED;
	int protoTableIndex = 0;

	setParserMode(PARSING_PROTOINSTANCE);
	curProtoInsStackInd++;

	#ifdef X3DPARSERVERBOSE
	printf ("parseProtoInstance, incremented curProtoInsStackInd to %d\n",curProtoInsStackInd);
	#endif

	currentProtoInstance = INT_ID_UNDEFINED;
	

	for (count = 0; atts[count]; count += 2) {
		#ifdef X3DPARSERVERBOSE
		TTY_SPACE
		printf ("parseProtoInstance: field %d , :%s=%s\n", count, atts[count], atts[count + 1]);
		#endif

		if (strcmp("name",atts[count]) == 0) {
			nameIndex=count+1;
		} else if (strcmp("containerField",atts[count]) == 0) {
			containerIndex = count+1;
		} else if (strcmp("DEF",atts[count]) == 0) {
			defNameIndex = count+1;

		} else if (strcmp("class",atts[count]) == 0) {
			ConsoleMessage ("field \"class\" not currently used in a ProtoInstance parse... sorry");
		} else if (strcmp("USE",atts[count]) == 0) {
			ConsoleMessage ("field \"USE\" not currently used in a ProtoInstance parse... sorry");
		}
		
	}
	#ifdef X3DPARSERVERBOSE
	printf ("...end of attributes\n");
	#endif

	/* did we have a containerField? */
	if (containerIndex != INT_ID_UNDEFINED) {
		containerField = findFieldInFIELDNAMES(atts[containerIndex]);
		/* printf ("parseProtoInstance, found a containerField of %s, id is %d\n",atts[containerIndex],containerField); */
	}

	/* so, the container will either be -1, or will have a valid FIELDNAMES index */
	ProtoInstanceTable[curProtoInsStackInd].container = containerField;

	/* did we have a DEF in the instance? */
	if (defNameIndex == INT_ID_UNDEFINED) ProtoInstanceTable[curProtoInsStackInd].defName = NULL;
	else ProtoInstanceTable[curProtoInsStackInd].defName = STRDUP(atts[defNameIndex]);

	/* did we find the name? */
	if (nameIndex != INT_ID_UNDEFINED) {
		#ifdef X3DPARSERVERBOSE
		printf ("parseProtoInstance, found name :%s:\n",atts[nameIndex]);
		#endif

		/* go through the PROTO table, and find the match, if it exists */
		for (protoTableIndex = 0; protoTableIndex <= currentProtoDeclare; protoTableIndex ++) {
			if (strcmp(atts[nameIndex],PROTONames[protoTableIndex].name) == 0) {
				currentProtoInstance = protoTableIndex;
				return;
			}
		}
	
	} else {
		ConsoleMessage ("\"ProtoInstance\" found, but field \"name\" not found!\n");
	}

	/* initialize this call level */
printf ("getProtoValue, curProtoInsStackInd %d, MAX %d\n",curProtoInsStackInd, PROTOINSTANCE_MAX_LEVELS);
	if ((curProtoInsStackInd < 0) || (curProtoInsStackInd >= PROTOINSTANCE_MAX_LEVELS)) {
		ConsoleMessage ("too many levels of ProtoInstances, recompile with PROTOINSTANCE_MAX_LEVELS higher ");
		curProtoInsStackInd = 0;
	}

	ProtoInstanceTable[curProtoInsStackInd].paircount = 0;
	#ifdef X3DPARSERVERBOSE
	printf("unsuccessful end parseProtoInstance\n");
	#endif
}

/*******************************************************************************************/

#define FIND_THE_CONNECT(mystr)  \
	connect = strstr(mystr,strCONNECT);  \
	/* printf ("	FIND_THE_CONNECT: connect string is:%s:\n",connect); */ \
	/* ok, we have a valid <IS> and </IS> lets process what is between them. */  \
	if (connect != NULL) {  \
	 mystr = connect + strlen (strCONNECT); }

#define FIND_NODE_PROTO_FIELD(mystr,ns,ps) \
	ns = strstr(mystr, NODEFIELD_EQUALS); \
	ps = strstr(mystr, PROTOFIELD_EQUALS); \
	if (ns!=NULL) ns += strlen(NODEFIELD_EQUALS); \
	if (ps!=NULL) ps += strlen(PROTOFIELD_EQUALS);

#define FILL_ID(mystr,myid) \
	{ char *dq;   \
		myid[0]='\0'; \
		if (mystr!=NULL) { \
			dq = strchr(mystr,'"'); \
			if (dq != NULL) { \
				*dq = '\0'; \
				strcpy(myid,mystr); \
				*dq = '"'; \
			} \
		} \
	}


/* substitute the IS connects in the protoInString with the values from the IS, and return the string */
/* protoInString = original string, with the <IS> copied, and spaced out */
/* IS - just a pointer into protoInString of where the IS was, before it was spaced out */
/* isString - the IS string that we have to substitute for */

static char* doISsubs(char *protoInString, char *IS, char *isString) {
	char *connect = NULL;
	char *ns = NULL;
	char *ps = NULL;
	char nodeFieldID[200];
	char protoFieldID[200];
	char tmp[200];
	char *newProtoInString;
	char *doobie = NULL;
	char ctmp;

	#ifdef X3DPARSERVERBOSE
		printf ("\nstart of doISsubs\n");
		printf ("doISsubs, orig :%s:\n",protoInString);
		printf ("IS pointer is at:%s:\n",IS);
		printf ("and the isString is :%s:\n",isString);
	#endif

	/* find a <connect>, and go through all <connect> strings until we have finished with this IS */
	FIND_THE_CONNECT(isString)
	while (connect != NULL) {
		char *foundNameEquals = NULL;

		#ifdef X3DPARSERVERBOSE
			printf ("doISsubs loop, find the connect:%s:\n",connect);
		#endif

		FIND_NODE_PROTO_FIELD(connect,ns,ps)
		FILL_ID(ns,nodeFieldID)
		FILL_ID(ps,protoFieldID)

		#ifdef X3DPARSERVERBOSE
			printf ("nodeFieldID :%s:  protoFieldID :%s:\n",nodeFieldID, protoFieldID);
		#endif

		/* What do we attach the nodeFieldID to?? lets go back and see what we can find */
		/* could this be a Script field, like:
			<field accessType="initializeOnly" name="imageName"> </field> ? */
		strcpy (tmp,"name=\"");
		strcat (tmp,nodeFieldID);
		strcat (tmp,"\"");

		#ifdef X3DPARSERVERBOSE
			printf ("trying to find the following :%s:\n",tmp);
		#endif

		foundNameEquals = strstr(protoInString,tmp);
		
		if (foundNameEquals != NULL) {
			printf ("found this, probably script field\n");
		} else {
			#ifdef X3DPARSERVERBOSE	
			printf ("did not find it, lets go back and find node to attach to\n"); 
			#endif


			/* we probably have something like this: 
			<ImageTexture>
                        	<IS>
                        		<connect nodeField='url' protoField='imageName'/>
					<connect nodeField='repeatS' protoField='yes'/>
                        	</IS>
                	</ImageTexture>

			which we want to turn into something like:
			<ImageTexture url=(substituted string) repeatS=(substituted string) />

			*/

			/* so, to do that, we have to take off the trailing ">" from the node name */
			ctmp = *IS;
			*IS='\0';
			doobie = strrchr(protoInString,'>');
			*IS = ctmp;
		
			if (doobie != NULL) {
				char *valueStr = NULL;

				/* ok, we know the location of the last ">", lets go and make a string with all
				   of this in it together */


				/* the value of the PROTO substitution field: */
				valueStr = getProtoValue(currentProtoInstance,protoFieldID);

				/* lets make up a new string long enough for the proto substitution */
				newProtoInString = MALLOC(strlen(valueStr) + strlen(protoInString) + 20);

				*doobie = '\0';
				strcpy (newProtoInString,protoInString);
				strcat (newProtoInString, " ");
				strcat (newProtoInString,nodeFieldID);
				strcat (newProtoInString,"='");

				/* now, is this possibly an MFString? */
				strcat (newProtoInString,valueStr); /* value from proto inv */

				strcat (newProtoInString,"'> ");
				/* any more additions go in here, so... */
				IS = newProtoInString + strlen(newProtoInString);

				*doobie=' ';
				strcat (newProtoInString, doobie);
				FREE_IF_NZ(protoInString);
				protoInString = newProtoInString;
			}
				
		}
		

		/* lets loop and do the next <connect> */
		connect += strlen (strCONNECT);
		FIND_THE_CONNECT(connect)

	}


	#ifdef X3DPARSERVERBOSE
		printf ("RETURNING :%s:\n",protoInString);
	#endif

	return protoInString;
}

/* have a </ProtoInstance> so should have valid name and fieldValues */
void expandProtoInstance(struct X3D_Group *myGroup) {
	int i;
	char *protoInString;
	int psSize;
	int rs;
	char *IS = NULL;
	char *endIS = NULL;
	int pf;


	#define OPEN_AND_READ_PROTO \
		PROTONames[currentProtoInstance].fileDescriptor = fopen (PROTONames[currentProtoInstance].fileName,"r"); \
		rs = fread(protoInString, 1, PROTONames[currentProtoInstance].charLen, PROTONames[currentProtoInstance].fileDescriptor); \
		protoInString[rs] = '\0'; /* ensure termination */ \
		fclose (PROTONames[currentProtoInstance].fileDescriptor); \
		/* printf ("OPEN AND READ %s returns:%s\n:\n",PROTONames[currentProtoInstance].fileName, protoInString); */ \
		if (rs != PROTONames[currentProtoInstance].charLen) { \
			ConsoleMessage ("protoInstance :%s:, expected to read %d, actually read %d\n",PROTONames[currentProtoInstance].name,  \
				PROTONames[currentProtoInstance].charLen,rs); \
		} 



	#define FIND_THE_END_OF_IS(mystr) \
		/* printf ("FIND_THE_END_OF_IS, input str :%s:\n",mystr); */ \
		endIS = strstr(mystr,strNOTIS); \
		/* printf ("	FIND_THE_END_OF_IS: endIS string is :%s:\n",endIS); */ \
		if (endIS == NULL) { \
			ConsoleMessage ("did not find an </IS> for ProtoInstance %s\n",PROTONames[currentProtoInstance].name); \
			FREE_IF_NZ(protoInString); FREE_IF_NZ(protoInString); \
			return; \
		} \
		endIS += strlen(strNOTIS); 

	#define FIND_THE_IS \
		 IS = strstr(protoInString,strIS); \
		/* printf ("	FIND_THE_IS: IS string is :%s:\n",IS); */ 

	#define ZERO_IS_TEXT_IN_ORIG \ 
		{ char *is = IS; \
		while ((is != endIS) && (*is != '\0')) { *is = ' '; is++; }}


	/* first, do we actually have a valid proto here? */
	if (currentProtoInstance == INT_ID_UNDEFINED) 
		return;

	#ifdef X3DPARSERVERBOSE
	printf ("ok, expandProtoInstance, have a valid protoInstance of %d\n",currentProtoInstance);
	#endif

	/* step 0. Does this one contain a DEF? */
	if (ProtoInstanceTable[curProtoInsStackInd].defName != NULL) {
		struct X3D_Node * me; 
		#ifdef X3DPARSERVERBOSE
			printf ("ProtoInstance, have a DEF, defining :%s: for node %u\n",
			ProtoInstanceTable[curProtoInsStackInd].defName,myGroup);
		#endif

		me = DEFNameIndex(ProtoInstanceTable[curProtoInsStackInd].defName,myGroup,FALSE);
		FREE_IF_NZ(ProtoInstanceTable[curProtoInsStackInd].defName);
	}

	/* step 1. read in the PROTO text. */
	psSize = PROTONames[currentProtoInstance].charLen * 10;

	if (psSize < 0) {
		ConsoleMessage ("problem with psSize in expandProtoInstance");
		return;
	}

	protoInString = MALLOC (psSize);
	protoInString = MALLOC(PROTONames[currentProtoInstance].charLen+1);
	protoInString[0] = '\0';

	/* read in the PROTO into the "protoInString" */
	OPEN_AND_READ_PROTO

	#ifdef X3DPARSERVERBOSE
	printf ("expandProtoInstance, now, we have in memory:\n%s:\n", protoInString);
	#endif


	/* loop through, and replace any IS'd fields with our PROTO expansion stuff... */
	FIND_THE_IS

	while (IS != NULL) {
		char *isString;
		unsigned int islen;

		FIND_THE_END_OF_IS(IS)

		if (endIS == NULL) {
			ConsoleMessage ("did not find </IS> in PROTO expansion");
			return;
		}

		/* copy the IS text over */
		islen =  (endIS - IS);
		isString = MALLOC(islen + 10);
		memcpy (isString,IS,islen);
		isString[islen] = '\0';
		
		#ifdef X3DPARSERVERBOSE
			printf ("is text is :%s:\n",isString);
		#endif

		/* zero the original data between the IS and the ENDIS */
		ZERO_IS_TEXT_IN_ORIG

		/* do IS substitutions */
		protoInString = doISsubs(protoInString,IS,isString);

		/* and keep doing this, until we have not more <IS> fields */
		FIND_THE_IS		
	}

	#ifdef X3DPARSERVERBOSE
	printf ("PROTO EXPANSION IS:\n%s\n:\n",protoInString);
	#endif

	/* parse this string */

	if (X3DParse (myGroup,protoInString)) {
		#ifdef X3DPARSERVERBOSE
		printf ("PARSED OK\n");
		#endif

		if (ProtoInstanceTable[curProtoInsStackInd].container == INT_ID_UNDEFINED) 
			pf = FIELDNAMES_children;
		else
			pf = ProtoInstanceTable[curProtoInsStackInd].container;
		myGroup->_defaultContainer = pf;
	
		#ifdef X3DPARSERVERBOSE
		printf ("expandProtoInstance cpsi %d, the proto's container is %s and as an id %d\n", curProtoInsStackInd, 
			FIELDNAMES[pf],myGroup->_defaultContainer);
		#endif
	} else {
		#ifdef X3DPARSERVERBOSE
		printf ("DID NOT PARSE THAT WELL:\n%s\n:\n",protoInString);
		#endif
	}

	/* remove the ProtoInstance calls from this stack */
	for (i=0; i<ProtoInstanceTable[curProtoInsStackInd].paircount; i++) {
		FREE_IF_NZ (ProtoInstanceTable[curProtoInsStackInd].name[i]);
		FREE_IF_NZ (ProtoInstanceTable[curProtoInsStackInd].value[i]);
	}
	ProtoInstanceTable[curProtoInsStackInd].paircount = 0;

	#ifdef X3DPARSERVERBOSE
	printf ("expandProtoInstance: decrementing curProtoInsStackInd from %d\n",curProtoInsStackInd);
	#endif

	linkNodeIn();
	DECREMENT_PARENTINDEX
	curProtoInsStackInd--;
        FREE_IF_NZ(protoInString);
}


void parseProtoBody (const char **atts) {
	#ifdef X3DPARSERVERBOSE
	TTY_SPACE
	printf ("start of parseProtoBody\n");
	#endif

	setParserMode(PARSING_PROTOBODY);
}

void parseProtoDeclare (const char **atts) {
	int count;
	int nameIndex = INT_ID_UNDEFINED;

	/* increment the currentProtoDeclare field. Check to see how many PROTOS we (bounds check) */
	currentProtoDeclare++;
	curProDecStackInd++;

	setParserMode(PARSING_PROTODECLARE);
	
	for (count = 0; atts[count]; count += 2) {
		#ifdef X3DPARSERVERBOSE
		TTY_SPACE
		printf ("parseProtoDeclare: field:%s=%s\n", atts[count], atts[count + 1]);
		#endif

		if (strcmp("name",atts[count]) == 0) {nameIndex=count+1;}
		else if ((strcmp("appinfo", atts[count]) != 0)  ||
			(strcmp("documentation",atts[count]) != 0)) {
			#ifdef X3DPARSERVERBOSE
			ConsoleMessage ("found field :%s: in a ProtoDeclare -skipping",atts[count]);
			#endif
		}
	}

	/* did we find the name? */
	if (nameIndex != INT_ID_UNDEFINED) {
		/* found it, lets open a new PROTO for this one */
		registerProto(atts[nameIndex]);
	} else {
		ConsoleMessage ("\"ProtoDeclare\" found, but field \"name\" not found!\n");
	}
}

/* simple sanity check, and change mode */
void parseProtoInterface (const char **atts) {
	if (getParserMode() != PARSING_PROTODECLARE) {
		ConsoleMessage ("got a <ProtoInterface>, but not within a <ProtoDeclare>\n");
	}
	setParserMode(PARSING_PROTOINTERFACE);
}

/* parse a script or proto field. Note that they are in essence the same, just used differently */
void parseScriptProtoField(struct VRMLLexer* myLexer, const char **atts) {
	int i;
	uintptr_t myScriptNumber = 0;
	int myparams[MPFIELDS];
	int which;
	int myFieldNumber;
	char *myValueString = NULL;
	int myAccessType;
	struct Shader_Script *myObj = NULL;
	int inShaderScript = FALSE;

	#ifdef X3DPARSERVERBOSE
	printf ("start of parseScriptProtoField\n");
	#endif

	/* configure internal variables, and check sanity for top of stack This should be a Script node */
	if (getParserMode() == PARSING_SCRIPT) {
		#ifdef X3DPARSERVERBOSE
		printf ("verifying: parseScriptProtoField, expecting a scriptish, got a %s\n",
			stringNodeType(parentStack[parentIndex]->_nodeType));
		#endif

		switch (parentStack[parentIndex]->_nodeType) {
			case NODE_Script: {
				struct X3D_Script* myScr = NULL;
				myScr = X3D_SCRIPT(parentStack[parentIndex]);
				myObj = (struct Shader_Script *) myScr->__scriptObj;
				myScriptNumber = myObj->num;
				break; }
			case NODE_ComposedShader: {
				struct X3D_ComposedShader* myScr = NULL;
				myScr = X3D_COMPOSEDSHADER(parentStack[parentIndex]);
				myObj = (struct Shader_Script *) myScr->__shaderObj;
				break; }
			case NODE_ShaderProgram: {
				struct X3D_ShaderProgram* myScr = NULL;
				myScr = X3D_SHADERPROGRAM(parentStack[parentIndex]);
				myObj = (struct Shader_Script *) myScr->__shaderObj;
				break; }
			case NODE_PackagedShader: {
				struct X3D_PackagedShader* myScr = NULL;
				myScr = X3D_PACKAGEDSHADER(parentStack[parentIndex]);
				myObj = (struct Shader_Script *) myScr->__shaderObj;
				break; }

			default: {

				ConsoleMessage("got an error on parseScriptProtoField, do not know how to handle a %s",
					stringNodeType(parentStack[parentIndex]->_nodeType));
				return;
			}
		}
		inShaderScript = TRUE;

	} else {
		myScriptNumber = currentProtoDeclare;

		#ifdef X3DPARSERVERBOSE
		TTY_SPACE
		printf ("parseScriptProtoField, working on proto idNumber %d\n",myScriptNumber);
		#endif
	}


	/* set up defaults for field parsing */
	for (i=0;i<MPFIELDS;i++) myparams[i] = INT_ID_UNDEFINED;
	

	/* copy the fields over */
	/* have a "key" "value" pairing here. They can be in any order; put them into our order */
	for (i = 0; atts[i]; i += 2) {
		#ifdef X3DPARSERVERBOSE
		TTY_SPACE
		printf ("parseScriptProtoField, field %d, looking at %s=\"%s\"\n",i,atts[i],atts[i+1]);
		#endif

		/* skip any "appinfo" or "documentation" fields here */
		if ((strcmp("appinfo", atts[i]) != 0)  ||
			(strcmp("documentation",atts[i]) != 0)) {
			if (strcmp(atts[i],"name") == 0) { which = MP_NAME;
			} else if (strcmp(atts[i],"accessType") == 0) { which = MP_ACCESSTYPE;
			} else if (strcmp(atts[i],"type") == 0) { which = MP_TYPE;
			} else if (strcmp(atts[i],"value") == 0) { which = MP_VALUE;
			} else {
				ConsoleMessage ("X3D Proto/Script parsing line %d: unknown field type %s",LINE,atts[i]);
				return;
			}
			if (myparams[which] != INT_ID_UNDEFINED) {
				ConsoleMessage ("X3DScriptParsing line %d, field %s already defined in this Script/Proto",
					LINE,atts[i],atts[i+1]);
			}
	
			/* record the index for the value of this field */
			myparams[which] = i+1;
		}
	}

	#ifdef X3DPARSERVERBOSE
	printf ("ok, fields copied over\n");
	printf ("myparams:name ind: %d accessType %d type: %d value %d\n",myparams[MP_NAME],myparams[MP_ACCESSTYPE],myparams[MP_TYPE], myparams[MP_VALUE]);
	printf ("and the values are: ");
	if (myparams[MP_NAME] != ID_UNDEFINED) printf ("name:%s ", atts[myparams[MP_NAME]]);
	if (myparams[MP_ACCESSTYPE] != ID_UNDEFINED) printf ("access:%s ", atts[myparams[MP_ACCESSTYPE]]);
	if (myparams[MP_TYPE] != ID_UNDEFINED) printf ("type:%s ", atts[myparams[MP_TYPE]]);
	if (myparams[MP_VALUE] != ID_UNDEFINED) printf ("value:%s ", atts[myparams[MP_VALUE]]);
	printf ("\n");
	#endif

	/* ok now, we have a couple of checks to make here. Do we have all the parameters required? */
	if (myparams[MP_NAME] == INT_ID_UNDEFINED) {
		ConsoleMessage("have a Script/PROTO at line %d with no parameter name",LINE);
		return;
	}
	if (myparams[MP_TYPE] == INT_ID_UNDEFINED) {
		ConsoleMessage("have a Script/PROTO at line %d with no parameter type",LINE);
		return;
	}

	if (myparams[MP_ACCESSTYPE] == INT_ID_UNDEFINED) {
		ConsoleMessage ("have a Script/PROTO at line %d with no paramater accessType ",LINE);
		return;
	}

	if (myparams[MP_VALUE] != INT_ID_UNDEFINED) myValueString = atts[myparams[MP_VALUE]];

	/* register this field with the Javascript Field Indexer */
	myFieldNumber = JSparamIndex((char *)atts[myparams[MP_NAME]],(char *)atts[myparams[MP_TYPE]]);


	/* convert eventIn, eventOut, field, and exposedField to new names */
	myAccessType = findFieldInPROTOKEYWORDS(atts[myparams[MP_ACCESSTYPE]]);
	switch (myAccessType) {
		case PKW_eventIn: myAccessType = PKW_inputOnly; break;
		case PKW_eventOut: myAccessType = PKW_outputOnly; break;
		case PKW_exposedField: myAccessType = PKW_inputOutput; break;
		case PKW_field: myAccessType = PKW_initializeOnly; break;
		default: {}
	}
	
	/* printf ("field parsing, so we have script %d, accessType %d fieldnumber %d, need to get field value\n",
		myScriptNumber, myAccessType, myFieldNumber); */

	if (inShaderScript) {
		struct ScriptFieldDecl* sdecl = NULL;
		indexT name = ID_UNDEFINED;
		union anyVrml defaultVal;

		/* get the name index */
        	lexer_fromString(myLexer,STRDUP(atts[myparams[MP_NAME]]));

		/* put it on the right Vector in myLexer */
    		switch(myAccessType) {
			#define LEX_DEFINE_FIELDID(suff) \
			   case PKW_##suff: \
			    if(!lexer_define_##suff(myLexer, &name)) \
			     ConsoleMessage ("Expected fieldNameId after field type!"); \
			    break;

	        LEX_DEFINE_FIELDID(initializeOnly)
	        LEX_DEFINE_FIELDID(inputOnly)
	        LEX_DEFINE_FIELDID(outputOnly)
	        LEX_DEFINE_FIELDID(inputOutput)
                default:
			ConsoleMessage ("define fieldID, unknown access type, %d\n",myAccessType);
			return;
    		}

		/* so, inputOnlys and outputOnlys DO NOT have initialValues, 
		   inputOutput and initializeOnly DO */
		if ((myAccessType == PKW_initializeOnly) || (myAccessType == PKW_inputOutput)) {
			if (myValueString == NULL) {
				ConsoleMessage ("Field, an initializeOnly or inputOut needs an initialValue");
				return;
			}
		} else {
			if (myValueString != NULL) {
				ConsoleMessage ("Field, an inputOnly or outputOnly can not have an initialValue");
				return;
			}
		}
				

		/* create a new scriptFieldDecl */
		sdecl = newScriptFieldDecl(myLexer,(indexT) myAccessType, 
			findFieldInFIELDTYPES(atts[myparams[MP_TYPE]]),  name);


		/* for now, set the value  -either the default, or not... */
		if (myValueString != NULL) {
			Parser_scanStringValueToMem(X3D_NODE(&defaultVal), 0, sdecl->fieldDecl->type, myValueString, TRUE);
		}
		scriptFieldDecl_setFieldValue(sdecl, defaultVal);

		
		/* fill in the string name and type */
		sdecl->ASCIIname=STRDUP(atts[myparams[MP_NAME]]);
		sdecl->ASCIItype=STRDUP(atts[myparams[MP_TYPE]]);

		/* add this field to the script */
		script_addField(myObj,sdecl);

	} else {

		registerX3DProtoField(myScriptNumber,
			findFieldInFIELDTYPES(atts[myparams[MP_TYPE]]),
			myAccessType,
			myFieldNumber,atts[myparams[MP_NAME]],myValueString);
	}

#ifdef OLDCODE
	/* and initialize it if a Script */
	if (getParserMode() == PARSING_SCRIPT) {
		/* parse this string value into a anyVrml union representation */
		if (myValueString != NULL)
			Parser_scanStringValueToMem(X3D_NODE(&value), 0, findFieldInFIELDTYPES(atts[myparams[MP_TYPE]]), myValueString, TRUE);
		else
			parseScriptFieldDefaultValue(findFieldInFIELDTYPES(atts[myparams[MP_TYPE]]), &value);
		
		/* send in the script field for initialization */
		SaveScriptField (myScriptNumber, myAccessType, findFieldInFIELDTYPES(atts[myparams[MP_TYPE]]),atts[myparams[MP_NAME]],value);
	}
#endif
}


/* we get script text from a number of sources; from the URL field, from CDATA, from a file
   pointed to by the URL field, etc... handle the data in one place */

void initScriptWithScript() {
	uintptr_t myScriptNumber;
	char *startingIndex;
	struct X3D_Script * me;
	char *myText = NULL;
	char *mypath;
	char *thisurl;
	int count;
	char filename[1000];
	int fromFile = FALSE;
	int removeIt = FALSE;

	struct Shader_Script *myObj;

	/* semantic checking... */
	me = X3D_SCRIPT(parentStack[parentIndex]);
	myObj = (struct Shader_Script *) me->__scriptObj;

	if (me->_nodeType != NODE_Script) {
		ConsoleMessage ("initScriptWithScript - Expected to find a NODE_Script, got a %s\n",
		stringNodeType(me->_nodeType));
		return;
	}

	#ifdef X3DPARSERVERBOSE
	printf ("endElement: CDATA_Text is %s\n",CDATA_Text);
	#endif

	myScriptNumber = myObj->num;

	/* did the script text come from a CDATA node?? */
	if (CDATA_Text != NULL) if (CDATA_Text[0] != '\0') myText = CDATA_Text;


	/* is this CDATA text? */
	if (myText != NULL) {
		struct Multi_String strHolder;

		strHolder.p = MALLOC (sizeof(struct Uni_String)*1);
		strHolder.p[0] = newASCIIString(myText);
		strHolder.n=1; 
		script_initCodeFromMFUri(me, &strHolder);
		FREE_IF_NZ(strHolder.p[0]->strptr);
		FREE_IF_NZ(strHolder.p);
	} else {
		script_initCodeFromMFUri(me, &X3D_SCRIPT(me)->url);
	}

	/* finish up here; if we used the CDATA area, set its length to zero */
	if (myText != NULL) CDATA_Text_curlen=0;

	setParserMode(PARSING_NODES);
	#ifdef X3DPARSERVERBOSE
	printf ("endElement: got END of script - script should be registered\n");
	#endif
}

void addToProtoCode(const char *name) {
        if (PROTONames[currentProtoDeclare].fileOpen) 
                PROTONames[currentProtoDeclare].charLen += fprintf (PROTONames[currentProtoDeclare].fileDescriptor,"</%s>\n",name);
}

void endProtoDeclare(void) {

		#ifdef X3DPARSERVERBOSE
		TTY_SPACE
		printf ("endElement, end of ProtoDeclare %d stack %d\n",currentProtoDeclare,curProDecStackInd);
		#endif

		/* decrement the protoDeclare stack count. If we are nested, get out of the nesting */
		curProDecStackInd--;

		/* now, a ProtoDeclare should be within normal nodes; make the expected mode PARSING_NODES, assuming
		   we don't have nested ProtoDeclares  */
		if (curProDecStackInd == 0) setParserMode(PARSING_NODES);

		if (curProDecStackInd < 0) {
			ConsoleMessage ("X3D_Parser found too many end ProtoDeclares at line %d\n",LINE);
			curProDecStackInd = 0; /* reset it */
		}
}

/* we are doing a ProtoInstance, and we do not have a fieldValue in the Instance for a parameter. See if it is
   available in the ProtoInterface. */
int getFieldValueFromProtoInterface (char *fieldName, int protono, char **value) {
	int ctr;
	struct Uni_String *tmp;
	int len;


	#ifdef X3DPARSERVERBOSE
	printf ("getFieldValueFromProtoInterface, looking for :%s: in proto %d\n",fieldName, protono);
	#endif
	

	len = strlen(fieldName) +1; /* len in Uni_String has the '\0' on it */
	

        for (ctr=0; ctr<=ScriptFieldTableSize; ctr++) {
		if (protono == ScriptFieldNames[ctr].scriptNumber) {
                	tmp = ScriptFieldNames[ctr].fieldName;
                	if (strcmp(fieldName,tmp->strptr)==0) {
				/* does the value for this one exist? */
				if (ScriptFieldNames[ctr].value == NULL) {
					*value = "";
					return TRUE;
				}

				*value = ScriptFieldNames[ctr].value->strptr;
				return TRUE;
			}
		}
	}

	/* did not find it. */
	*value = "";
	return FALSE;
}

/* record each field of each script - the type, kind, name, and associated script */
void registerX3DProtoField(int myScriptNumber,int type,int kind, int myFieldOffs, char *name, char *value) {

	/* semantic check */
	if ((getParserMode() != PARSING_SCRIPT) && (getParserMode() != PARSING_PROTOINTERFACE)) {
		ConsoleMessage ("registerX3DProtoField: wrong mode - got %d\n",getParserMode());
	}

	ScriptFieldTableSize ++;

	#ifdef X3DPARSERVERBOSE
	printf ("registering script field %s script %d index %d\n",name,myScriptNumber,ScriptFieldTableSize);
	printf ("	type %d kind %d fieldOffs %d\n",type,kind,myFieldOffs);
	#endif


	/* ok, we got a name and a type */
	if (ScriptFieldTableSize >= MAXScriptFieldParams) {
		/* oooh! not enough room at the table */
		MAXScriptFieldParams += 100; /* arbitrary number */
		ScriptFieldNames = (struct ScriptFieldStruct*)REALLOC (ScriptFieldNames, sizeof(*ScriptFieldNames) * MAXScriptFieldParams);
	}

	ScriptFieldNames[ScriptFieldTableSize].scriptNumber = myScriptNumber;
	ScriptFieldNames[ScriptFieldTableSize].fieldName = newASCIIString(name);
	if (value == NULL) ScriptFieldNames[ScriptFieldTableSize].value = NULL;
	else ScriptFieldNames[ScriptFieldTableSize].value = newASCIIString(value);
	ScriptFieldNames[ScriptFieldTableSize].fromScriptNotPROTO = getParserMode() == PARSING_SCRIPT;
	ScriptFieldNames[ScriptFieldTableSize].type = type;
	ScriptFieldNames[ScriptFieldTableSize].kind = kind;
	ScriptFieldNames[ScriptFieldTableSize].offs = myFieldOffs;
}


/* look through the script fields for this field, and return the values. */
int getFieldFromScript (struct VRMLLexer *myLexer, char *fieldName, struct Shader_Script *me, int *offs, int *type, int *accessType) {
	int ctr;
	struct Uni_String *tmp;
	int len;
	struct ScriptFieldDecl* myField = NULL;
	const char** userArr;
	size_t userCnt;
	indexT retUO = ID_UNDEFINED;

	#ifdef X3DPARSERVERBOSE
	printf ("getFieldFromScript, looking for %s\n",fieldName);
	#endif

	/* go through the user arrays in this lexer, and see if we have a match */


	#ifdef CPROTOVERBOSE
	printf ("getProtoFieldDeclaration, for field :%s:\n",thisID);
	#endif


#define LOOK_FOR_FIELD_IN(whicharr) \
	if (myField == NULL) { \
	userArr=&vector_get(const char*, myLexer->user_##whicharr, 0); \
	userCnt=vector_size(myLexer->user_##whicharr);\
	retUO=findFieldInARR(fieldName, userArr, userCnt);\
	if (retUO != ID_UNDEFINED) { \
		myField=script_getField(me,retUO,PKW_##whicharr); \
	}}

	LOOK_FOR_FIELD_IN(initializeOnly);
	LOOK_FOR_FIELD_IN(inputOnly);
	LOOK_FOR_FIELD_IN(outputOnly);
	LOOK_FOR_FIELD_IN(inputOutput);


	if (myField != NULL) {
		int myFieldNumber;

		/* is this a script? if so, lets do the conversion from our internal lexer name index to
		   the scripting name index. */
		if (me->ShaderScriptNode->_nodeType == NODE_Script) {
			/* wow - have to get the Javascript text string index from this one */
			myFieldNumber = JSparamIndex(fieldName,stringFieldtypeType(myField->fieldDecl->type)); 
			*offs=myFieldNumber;


		} else {
			*offs = myField->fieldDecl->name;
		}
		*type = myField->fieldDecl->type;
		/* go from PKW_xxx to KW_xxx  .... sigh ... */
		*accessType = mapToKEYWORDindex(myField->fieldDecl->mode);
		return TRUE;
	}

	#ifdef X3DPARSERVERBOSE
	printf ("getFieldFromScript, did not find field %s in script\n",fieldName);
	#endif
	
	/* did not find it */
	*offs = INT_ID_UNDEFINED;  *type = 0;
	return FALSE;
}
