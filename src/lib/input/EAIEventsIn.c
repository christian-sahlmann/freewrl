/*
=INSERT_TEMPLATE_HERE=

$Id: EAIEventsIn.c,v 1.6 2008/12/10 14:31:53 couannette Exp $

Handle incoming EAI (and java class) events with panache.

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"

#include "../input/EAIheaders.h"
#include "../world_script/fieldSet.h"

#include <ctype.h> /* FIXME: config armor */


#define EAI_BUFFER_CUR EAIbuffer[bufPtr]

/* used for loadURL */
struct X3D_Anchor EAI_AnchorNode;
int waiting_for_anchor = FALSE;

void createLoadURL(char *bufptr);
void makeFIELDDEFret(uintptr_t,char *buf,int c);
void handleRoute (char command, char *bufptr, char *buf, int repno);
void handleGETNODE (char *bufptr, char *buf, int repno);
void handleGETROUTES (char *bufptr, char *buf, int repno);


/******************************************************************************
*
* EAI_parse_commands
*
* there can be many commands waiting, so we loop through commands, and return
* a status of EACH command
*
* a Command starts off with a sequential number, a space, then a letter indicating
* the command, then the parameters of the command.
*
* the command names are #defined at the start of this file.
*
* some commands have sub commands (eg, get a value) to indicate data types,
* (eg, EAI_SFFLOAT); these sub types are indicated with a lower case letter; again,
* look to the top of this file for the #defines
*
*********************************************************************************/


void EAI_parse_commands () {
	char buf[EAIREADSIZE];	/* return value place*/
	char ctmp[EAIREADSIZE];	/* temporary character buffer*/
	char dtmp[EAIREADSIZE];	/* temporary character buffer*/
	uintptr_t nodarr[200]; /* returning node/backnode combos from CreateVRML fns.*/

	int count;
	char command;
	int perlNode; uintptr_t cNode;
	int bufPtr = 0;		/* where we are in the EAI input buffer */
	
	uintptr_t ra,rb,rc,rd;	/* temps*/
	int tmp_a, tmp_b, tmp_c, tmp_d;

	unsigned int scripttype;
	char *EOT;		/* ptr to End of Text marker*/
	int retint;		/* used for getting retval for sscanf */
	int flag;

	struct X3D_Node *boxptr;
        int ctype;
	int xxx;

	while (EAI_BUFFER_CUR> 0) {
		if (eaiverbose) {
			printf ("EAI_parse_commands:start of while loop, strlen %d str :%s:\n",strlen((&EAI_BUFFER_CUR)),(&EAI_BUFFER_CUR));
		}

		/* step 1, get the command sequence number */
		if (sscanf ((&EAI_BUFFER_CUR),"%d",&count) != 1) {
			printf ("EAI_parse_commands, expected a sequence number on command :%s:\n",(&EAI_BUFFER_CUR));
			count = 0;
		}
		if (eaiverbose) {
			printf ("EAI - seq number %d\n",count);
		}

		/* step 2, skip past the sequence number */
		while (isdigit(EAI_BUFFER_CUR)) bufPtr++;
		/* if (eaiverbose) {
			printf("past sequence number, string:%s\n",(&EAI_BUFFER_CUR));
		} */

		while (EAI_BUFFER_CUR == ' ') bufPtr++;
		/* if (eaiverbose) {
			printf ("past the space, string:%s\n",(&EAI_BUFFER_CUR)); 
		} */

		/* step 3, get the command */

		command = EAI_BUFFER_CUR;
		if (eaiverbose) {
			printf ("command %c strlen %d\n",command,strlen(&EAI_BUFFER_CUR));
		}
		bufPtr++;

		/* return is something like: $hand->print("RE\n$reqid\n1\n$id\n");*/
		if (eaiverbose) {
			printf ("\n... %d ",count);
		}

		switch (command) {
			case GETRENDPROP: {
				if (eaiverbose) {
					printf ("GETRENDPROP\n");
				}

				/* is MultiTexture initialized yet? */
				if (maxTexelUnits < 0) init_multitexture_handling();

				sprintf (buf,"RE\n%f\n%d\n%s %dx%d %d %s %d %f",TickTime,count,
					"SMOOTH",				/* Shading */
					global_texSize, global_texSize, 	/* Texture size */	
					maxTexelUnits,				/* texture units */
					"FALSE",				/* antialiased? */
					displayDepth,				/* bit depth of display */
					256.0					/* amount of memory left on card -
										   can not find this in OpenGL, so
										   just make it large... */
					);
				break;
				}

			case GETNAME: {
				if (eaiverbose) {
					printf ("GETNAME\n");
				}
				sprintf (buf,"RE\n%f\n%d\n%s",TickTime,count,BrowserName);
				break;
				}
			case GETVERSION: {
				if (eaiverbose) {
					printf ("GETVERSION\n");
				}
				sprintf (buf,"RE\n%f\n%d\n%s",TickTime,count,libFreeX3D_get_version());
				break;
				}
			case GETENCODING: {
				if (eaiverbose) {
					printf ("GETENCODING\n");
				}
				sprintf (buf,"RE\n%f\n%d\n%d",TickTime,count,currentFileVersion);
				break;
				}
			case GETCURSPEED: {
				if (eaiverbose) {
					printf ("GETCURRENTSPEED\n");
				}
				/* get the BrowserSpeed variable updated */
				getCurrentSpeed();
				sprintf (buf,"RE\n%f\n%d\n%f",TickTime,count,BrowserSpeed);
				break;
				}
			case GETFRAMERATE: {
				if (eaiverbose) {
					printf ("GETFRAMERATE\n");
				}	
				sprintf (buf,"RE\n%f\n%d\n%f",TickTime,count,BrowserFPS);
				break;
				}
			case GETURL: {
				if (eaiverbose) {	
					printf ("GETURL\n");
				}	
				sprintf (buf,"RE\n%f\n%d\n%s",TickTime,count,BrowserFullPath);
				break;
				}
			case GETNODE:  {
				handleGETNODE(&EAI_BUFFER_CUR,buf,count);
				break;
			}
			case GETROUTES:  {
				handleGETROUTES(&EAI_BUFFER_CUR,buf,count);
				break;
			}

			case GETEAINODETYPE: {
				handleGETEAINODETYPE(&EAI_BUFFER_CUR,buf,count);
				break;
			}

			case GETNODETYPE: {
				if (eaiverbose) {	
					printf ("GENODETYPE\n");
				}	
				retint = sscanf(&EAI_BUFFER_CUR,"%d",&cNode);
				if (cNode != 0) {
					boxptr = X3D_NODE(cNode);
					sprintf (buf,"RE\n%f\n%d\n%d",TickTime,count,getSAI_X3DNodeType (
						boxptr->_nodeType));
				} else {
					sprintf (buf,"RE\n%f\n%d\n-1",TickTime,count);
				}
				/* printf ("GETNODETYPE, for node %s, returns %s\n",(&EAI_BUFFER_CUR),buf); */
					
				break;
				}
			case GETFIELDTYPE:  {
				/*format int seq# COMMAND  int node#   string fieldname   string direction*/

				retint=sscanf (&EAI_BUFFER_CUR,"%d %d %s %s",&perlNode, &cNode, ctmp,dtmp);
				if (eaiverbose) {	
					printf ("GETFIELDTYPE cptr %d %s %s\n",cNode, ctmp, dtmp);
				}	

				EAI_GetType (cNode, ctmp, dtmp, &ra, &rb, &rc, &rd, &scripttype, &xxx);

				sprintf (buf,"RE\n%f\n%d\n%d %d %d %c %d %s",TickTime,count,ra,rb,rc,rd,
						scripttype,KEYWORDS[xxx]);
				break;
				}
			case SENDEVENT:   {
				/*format int seq# COMMAND NODETYPE pointer offset data*/
				if (eaiverbose) {	
					printf ("SENDEVENT, strlen %d\n",strlen(&EAI_BUFFER_CUR));
				}
				setField_FromEAI (&EAI_BUFFER_CUR);
				if (eaiverbose) {	
					printf ("after SENDEVENT, strlen %d\n",strlen(&EAI_BUFFER_CUR));
				}
				break;
				}
			case MIDIINFO: {
				if (eaiverbose) {	
					printf ("MIDIINFO %s\n",&EAI_BUFFER_CUR);
				}	

				EOT = strstr(&EAI_BUFFER_CUR,"\nEOT\n");
				/* if we do not have a string yet, we have to do this...*/
				while (EOT == NULL) {
					EAIbuffer = read_EAI_socket(EAIbuffer,&EAIbufcount, &EAIbufsize, &EAIlistenfd);
					EOT = strstr(&EAI_BUFFER_CUR,"\nEOT\n");
				}

				*EOT = 0; /* take off the EOT marker*/
				ReWireRegisterMIDI(&EAI_BUFFER_CUR);

				/* finish this for now - note the pointer math. */
				bufPtr = EOT+3-EAIbuffer;
				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
				}
			case MIDICONTROL: {
				if (eaiverbose) {	
					printf ("MIDICONTROL, %s\n",&EAI_BUFFER_CUR);
				}	
				/* sprintf (buf,"RE\n%f\n%d\n%d",TickTime,count, ReWireMIDIControl(&EAI_BUFFER_CUR)); */
				ReWireMIDIControl(&EAI_BUFFER_CUR);
				break;
				}
			case CREATEVU:
			case CREATEVS: {
				/*format int seq# COMMAND vrml text     string EOT*/
				if (command == CREATEVS) {
					if (eaiverbose) {	
						printf ("CREATEVS %s\n",&EAI_BUFFER_CUR);
					}	

					EOT = strstr(&EAI_BUFFER_CUR,"\nEOT\n");
					/* if we do not have a string yet, we have to do this...*/
					while (EOT == NULL) {
						EAIbuffer = read_EAI_socket(EAIbuffer,&EAIbufcount, &EAIbufsize, &EAIlistenfd);
						EOT = strstr(&EAI_BUFFER_CUR,"\nEOT\n");
					}

					*EOT = 0; /* take off the EOT marker*/

					ra = EAI_CreateVrml("String",(&EAI_BUFFER_CUR),nodarr,200);
					/* finish this, note the pointer maths */
					bufPtr = EOT+3-EAIbuffer;
				} else {
 					char *filename = (char *)MALLOC(1000);
					char *mypath;

					/* sanitize this string - remove leading and trailing garbage */
					rb = 0;
					while ((EAI_BUFFER_CUR!=0) && (EAI_BUFFER_CUR <= ' ')) bufPtr++;
					while (EAI_BUFFER_CUR > ' ') { ctmp[rb] = EAI_BUFFER_CUR; rb ++; bufPtr++; }

					/* ok, lets make a real name from this; maybe it is local to us? */
					ctmp[rb] = 0;

					/* get the current parent */
					mypath = STRDUP(ctmp);
					/* printf ("CREATEVU, mypath %s\n",mypath); */

					/* and strip off the file name, leaving any path */
					removeFilenameFromPath (mypath);
					/* printf ("CREATEVU, mypath sans file: %s\n",mypath); */

					/* add the two together */
					makeAbsoluteFileName(filename,mypath,ctmp);
					/* printf ("CREATEVU, filename, %s\n",filename); */

					if (eaiverbose) {	
						printf ("CREATEVU %s\n",filename);
					}	
					ra = EAI_CreateVrml("URL",filename,nodarr,200);
					FREE_IF_NZ(filename);
					FREE_IF_NZ(mypath);
				}

				sprintf (buf,"RE\n%f\n%d\n",TickTime,count);
				for (rb = 0; rb < ra; rb++) {
					/* printf ("create returns %d of %d %u\n",rb, ra, nodarr[rb]); */
					/* we have to skip the "perl" nodes, as they are not used and are
					   always going to be zero, but the real ones have to be EAIregistered */
					if (nodarr[rb] == 0)
						strcat (buf, "0 ");
					else {
						sprintf (ctmp,"%d ", registerEAINodeForAccess(X3D_NODE(nodarr[rb])));
						strcat (buf,ctmp);
					}
				}
				break;
				}

			case SENDCHILD :  {
				struct X3D_Node *node;
				uintptr_t *address;

				/*format int seq# COMMAND  int node#   ParentNode field ChildNode*/

				retint=sscanf (&EAI_BUFFER_CUR,"%d %d %s %d",&ra,&rb,ctmp,&rc);

				node = getEAINodeFromTable(ra);
				address = getEAIMemoryPointer (ra,rb);
				sprintf (dtmp,"%u",getEAINodeFromTable(rc));

				if (eaiverbose) {	
					printf ("SENDCHILD Parent: %d ParentField: %d %s Child: %s\n",ra, rb, ctmp, dtmp);
				}	

				/* add (1), remove (2) or replace (0) for the add/remove/set flag. But,
				   we only have addChildren or removeChildren, so flag can be 1 or 2 only */
				if (strcmp(ctmp,"removeChildren")==0) { flag = 2;} else {flag = 1;}


				getMFNodetype (dtmp,(struct Multi_Node *)address, node, flag);

				/* tell the routing table that this node is updated - used for RegisterListeners */
				MARK_EVENT(node,getEAIActualOffset(ra,rb));

				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
				}
			case REGLISTENER: {
				struct X3D_Node * node;
				int offset;

				if (eaiverbose) {	
					printf ("REGISTERLISTENER %s \n",&EAI_BUFFER_CUR);
				}	

				/*143024848 88 8 e 6*/
				retint=sscanf (&EAI_BUFFER_CUR,"%d %d %c %d",&tmp_a,&tmp_b,ctmp,&tmp_c);
				node = getEAINodeFromTable(tmp_a);
				offset = getEAIActualOffset(tmp_a,tmp_b);

				/* so, count = query id, tmp_a pointer, tmp_b, offset, ctmp[0] type, tmp_c, length*/
				ctmp[1]=0;

				if (eaiverbose) printf ("REGISTERLISTENER from %lu foffset %d fieldlen %d type %s \n",
						(uintptr_t)node, offset ,tmp_c,ctmp);


				/* put the address of the listener area in a string format for registering
				   the route - the route propagation will copy data to here */
				sprintf (EAIListenerArea,"%lu:0",(uintptr_t)&EAIListenerData);

				/* set up the route from this variable to the handle_Listener routine */
				CRoutes_Register  (1,node, offset, 1, EAIListenerArea, (int) tmp_c,(void *) 
					&handle_Listener, 0, (count<<8)+mapEAItypeToFieldType(ctmp[0])); /* encode id and type here*/
				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
				}

			case UNREGLISTENER: {
				struct X3D_Node * node;
				int offset;

				if (eaiverbose) {	
					printf ("UNREGISTERLISTENER %s \n",&EAI_BUFFER_CUR);
				}	

				/*143024848 88 8 e 6*/
				retint=sscanf (&EAI_BUFFER_CUR,"%d %d %c %d",&tmp_a,&tmp_b,ctmp,&tmp_c);
				node = getEAINodeFromTable(tmp_a);
				offset = getEAIActualOffset(tmp_a,tmp_b);

				/* so, count = query id, tmp_a pointer, tmp_b, offset, ctmp[0] type, tmp_c, length*/
				ctmp[1]=0;

				if (eaiverbose) printf ("UNREGISTERLISTENER from %lu foffset %d fieldlen %d type %s \n",
						(uintptr_t)node, offset ,tmp_c,ctmp);


				/* put the address of the listener area in a string format for registering
				   the route - the route propagation will copy data to here */
				sprintf (EAIListenerArea,"%lu:0",(uintptr_t)&EAIListenerData);

				/* set up the route from this variable to the handle_Listener routine */
				CRoutes_Register  (0,node, offset, 1, EAIListenerArea, (int) tmp_c,(void *) 
					&handle_Listener, 0, (count<<8)+mapEAItypeToFieldType(ctmp[0])); /* encode id and type here*/

				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
				}

			case GETVALUE: {
				handleEAIGetValue(command, &EAI_BUFFER_CUR,buf,count);
				break;
				}
			case REPLACEWORLD:  {
				if (eaiverbose) {	
					printf ("REPLACEWORLD %s \n",&EAI_BUFFER_CUR);
				}	

				EAI_RW(&EAI_BUFFER_CUR);
				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
				}

			case GETPROTODECL:  {
				if (eaiverbose) {	
					printf ("SAI SV ret command .%s\n",&EAI_BUFFER_CUR);
				}	
				sprintf (buf,"RE\n%f\n%d\n%s",TickTime,count,SAI_StrRetCommand ((char) command,&EAI_BUFFER_CUR));
				break;
				}
			case REMPROTODECL: 
			case UPDPROTODECL: 
			case UPDNAMEDNODE: 
			case REMNAMEDNODE:  {
				if (eaiverbose) {	
					printf ("SV int ret command ..%s\n",&EAI_BUFFER_CUR);
				}	
				sprintf (buf,"RE\n%f\n%d\n%d",TickTime,count,
					SAI_IntRetCommand ((char) command,&EAI_BUFFER_CUR));
				break;
				}
			case ADDROUTE:
			case DELETEROUTE:  {
				handleRoute (command, &EAI_BUFFER_CUR,buf,count);
				break;
				}

		  	case STOPFREEWRL: {
				if (eaiverbose) {	
					printf ("Shutting down Freewrl\n");
				}	
				if (!RUNNINGASPLUGIN) {
					doQuit();
				    break;
				}
			    }
			  case VIEWPOINT: {
				if (eaiverbose) {	
					printf ("Viewpoint :%s:\n",&EAI_BUFFER_CUR);
				}	
				/* do the viewpoints. Note the spaces in the strings */
				if (!strcmp(&EAI_BUFFER_CUR, " NEXT")) Next_ViewPoint();
				if (!strcmp(&EAI_BUFFER_CUR, " FIRST")) First_ViewPoint();
				if (!strcmp(&EAI_BUFFER_CUR, " LAST")) Last_ViewPoint();
				if (!strcmp(&EAI_BUFFER_CUR, " PREV")) Prev_ViewPoint();

				sprintf (buf,"RE\n%f\n%d\n0",TickTime,count);
				break;
			    }

			case LOADURL: {
				if (eaiverbose) {	
					printf ("loadURL %s\n",&EAI_BUFFER_CUR);
				}	

				/* signal that we want to send the Anchor pass/fail to the EAI code */
				waiting_for_anchor = TRUE;

				/* make up the URL from what we currently know */
				createLoadURL(&EAI_BUFFER_CUR);


				/* prep the reply... */
				sprintf (buf,"RE\n%f\n%d\n",TickTime,count);

				/* now tell the EventLoop that BrowserAction is requested... */
				AnchorsAnchor = &EAI_AnchorNode;
				BrowserAction = TRUE;
				break;
				}

			case CREATEPROTO: 
			case CREATENODE: {
				/* sanitize this string - remove leading and trailing garbage */
				rb = 0;
				while ((EAI_BUFFER_CUR!=0) && (EAI_BUFFER_CUR <= ' ')) bufPtr++;
				while (EAI_BUFFER_CUR > ' ') { ctmp[rb] = EAI_BUFFER_CUR; rb ++; bufPtr++; }

				ctmp[rb] = 0;
				if (eaiverbose) {	
					printf ("CREATENODE/PROTO %s\n",ctmp);
				}	

				/* set up the beginnings of the return string... */
				sprintf (buf,"RE\n%f\n%d\n",TickTime,count);

				if (command == CREATENODE) {
					if (eaiverbose) {	
						printf ("CREATENODE, %s is this a simple node? %d\n",ctmp,findFieldInNODES(ctmp));
					}	

					ctype = findFieldInNODES(ctmp);
					if (ctype > -1) {
						/* yes, use C only to create this node */
						sprintf (ctmp, "0 %d ",createNewX3DNode(ctype));
						strcat (buf,ctmp);
						/* set ra to 0 so that the sprintf below is not used */
						ra = 0;
					} else {
						ra = EAI_CreateVrml("CREATENODE",ctmp,nodarr,200); 
					}
				} else if (command == CREATEPROTO)
					ra = EAI_CreateVrml("CREATEPROTO",ctmp,nodarr,200); 
				else 
					printf ("eai - huh????\n");


				for (rb = 0; rb < ra; rb++) {
					sprintf (ctmp,"%d ", nodarr[rb]);
					strcat (buf,ctmp);
				}
				break;
				}

			case GETFIELDDEFS: {
				/* get a list of fields of this node */
				sscanf (&EAI_BUFFER_CUR,"%d",&ra);
				makeFIELDDEFret(ra,buf,count);
				break;
				}

			case GETNODEDEFNAME: {
				/* return a def name for this node. */
				sprintf (buf,"RE\n%f\n%d\n%s",TickTime,count,
					SAI_StrRetCommand ((char) command,&EAI_BUFFER_CUR));

				break;
				}

			default: {
				printf ("unhandled command :%c: %d\n",command,command);
				strcat (buf, "unknown_EAI_command");
				break;
				}

			}


		/* send the response - events don't send a reply */
		/* and, Anchors send a different reply (loadURLS) */
		if ((command != SENDEVENT) && (command != MIDICONTROL)) {
			if (command != LOADURL) strcat (buf,"\nRE_EOT");
			EAI_send_string (buf,EAIlistenfd);
		}

		/* printf ("end of command, remainder %d ",strlen(&EAI_BUFFER_CUR)); */
		/* skip to the next command */
		while (EAI_BUFFER_CUR >= ' ') bufPtr++;

		/* skip any new lines that may be there */
		while ((EAI_BUFFER_CUR == 10) || (EAI_BUFFER_CUR == 13)) bufPtr++;
		/* printf ("and %d : indx %d thread %d\n",strlen(&EAI_BUFFER_CUR),bufPtr,pthread_self()); */
	}
}

/* read in a C pointer, and field offset, and make sense of it */
int getEAINodeAndOffset (char *bufptr, struct X3D_Node **Node, int *FieldInt, int fromto) {
	int rv;
	char *x;
	char fieldTemp[2000];
	struct X3D_Node *sn;

	rv = TRUE;
	sscanf(bufptr, "%d", &fieldTemp); /* WARNING: buffer overflow */
	sn = getEAINodeFromTable(fieldTemp);
	*Node = sn;

	/* copy the from field */
	x = fieldTemp;
	while (*bufptr != ' ') bufptr++; while (*bufptr == ' ') bufptr++;
	while (*bufptr > ' ') { *x = *bufptr; x++; bufptr++;}
	*x = '\0';

	*FieldInt = findRoutedFieldInFIELDNAMES(sn,fieldTemp,fromto);

	if (*FieldInt < 0) {
		printf ("EAIEventsIn: trouble finding field %s of %s\n",
				fieldTemp, stringNodeType(sn->_nodeType));
		rv = FALSE;
	}	
	return rv;
}

void handleGETROUTES (char *bufptr, char *buf, int repno) {
	int numRoutes;
	int count;
	uintptr_t fromNode;
	uintptr_t toNode;
	int fromOffset;
	int toOffset;
	char  ctmp[200];
	
	sprintf (buf,"RE\n%f\n%d\n",TickTime,repno);

	numRoutes = getRoutesCount();

	if (numRoutes < 2) {
		strcat (buf,"0");
		return;
	}

	/* tell how many routes there are */
	sprintf (ctmp,"%d ",numRoutes-2);
	strcat (buf,ctmp);

	/* remember, in the routing table, the first and last entres are invalid, so skip them */
	for (count = 1; count < (numRoutes-1); count++) {
		getSpecificRoute (count,&fromNode, &fromOffset, &toNode, &toOffset);

		sprintf (ctmp, "%d %d %s %d %d %s ",0,fromNode,
			findFIELDNAMESfromNodeOffset(X3D_NODE(fromNode),fromOffset),
			0,toNode,
			findFIELDNAMESfromNodeOffset(X3D_NODE(toNode),toOffset)
			);
		strcat (buf,ctmp);
		/* printf ("route %d is:%s:\n",count,ctmp); */
	}

	/* printf ("getRoutes returns %s\n",buf); */
}

void handleGETNODE (char *bufptr, char *buf, int repno) {
	int retint;
	char ctmp[200];
	int mystrlen;
	int count;

	/*format int seq# COMMAND    string nodename*/

	retint=sscanf (bufptr," %s",ctmp);
	mystrlen = strlen(ctmp);

	if (eaiverbose) {	
		printf ("GETNODE %s\n",ctmp); 
	}	

	/* is this the SAI asking for the root node? */
	if (strcmp(ctmp,SYSTEMROOTNODE)) {
		sprintf (buf,"RE\n%f\n%d\n0 %d",TickTime,repno, EAI_GetNode(ctmp));
	} else {
		/* yep i this is a call for the rootNode */
		sprintf (buf,"RE\n%f\n%d\n0 %d",TickTime,repno, EAI_GetRootNode());
	}
	if (eaiverbose) {	
		printf ("GETNODE returns %s\n",buf); 
	}	
}

/* get the actual node type, whether Group, IndexedFaceSet, etc, and its DEF name, if applicapable */
void handleGETEAINODETYPE (char *bufptr, char *buf, int repno) {
	int retint;
	int nodeHandle;
	struct X3D_Node * myNode;
	int ctr;
	char *cptr;

	/*format int seq# COMMAND    string nodename*/

	retint=sscanf (bufptr," %d",&nodeHandle);
	myNode = getEAINodeFromTable(nodeHandle);

	if (myNode == NULL) {
		printf ("Internal EAI error, node %d not found\n",nodeHandle);
		sprintf (buf,"RE\n%f\n%d\n__UNDEFINED __UNDEFINED",TickTime,repno);
		return;
	}
		
	/* so, this is a valid node, lets find out if it is DEFined or whatever... */

        /* Try to get X3D node name */
	cptr = X3DParser_getNameFromNode(myNode);
	if (cptr != NULL) {
		sprintf (buf,"RE\n%f\n%d\n%s %s",TickTime,repno,stringNodeType(myNode->_nodeType), cptr);
		return;
	}

        /* Try to get VRML node name */
	cptr= parser_getNameFromNode(myNode);
	if (cptr != NULL) {
		sprintf (buf,"RE\n%f\n%d\n%s %s",TickTime,repno,stringNodeType(myNode->_nodeType), cptr);
		return;
	}

	/* no, this node is just undefined */
	sprintf (buf,"RE\n%f\n%d\n%s __UNDEFINED",TickTime,repno,stringNodeType(myNode->_nodeType));
}


/* add or delete a route */
void handleRoute (char command, char *bufptr, char *buf, int repno) {
	struct X3D_Node *fromNode;
	struct X3D_Node *toNode;
	char fieldTemp[2000];
	int fromFieldInt, toFieldInt;
	int *np;
	int fromOffset, fromVRMLtype, toOffset, toVRMLtype;
	int adrem;

	int rv;

	/* assume that all is ok right now */
	rv = TRUE;

	/* get ready for the reply */
	sprintf (buf,"RE\n%f\n%d\n",TickTime,repno);

	/* printf ("handleRoute, string %s\n",bufptr); */
	
	/* ------- worry about the route from section -------- */

	/* read in the fromNode pointer */
	while (*bufptr == ' ') bufptr++;
	if (!getEAINodeAndOffset (bufptr, &fromNode, &fromFieldInt,0)) rv = FALSE;

	/* skip past the first node & field, to get ready for the next one */
	while (*bufptr != ' ') bufptr++; while (*bufptr == ' ') bufptr++;
	while (*bufptr != ' ') bufptr++; while (*bufptr == ' ') bufptr++;

	/* printf ("handleRoute, to string %s\n",bufptr);
	printf ("fromNode is of type %s\n",stringNodeType(fromNode->_nodeType)); */

	/* ------- now, the route to section -------- */

	if (!getEAINodeAndOffset (bufptr, &toNode, &toFieldInt,1)) rv = FALSE;
	/* printf ("toNode is of type %s\n",stringNodeType(toNode->_nodeType)); */

	/* ------- can these routes work in these nodes?  -------- */
	if (rv == TRUE) {
		/* get the from field info */
		np = (int *)NODE_OFFSETS[fromNode->_nodeType];

		/* go through and find the entry for this field, looks like:
		const int OFFSETS_TimeSensor[] = {
		        FIELDNAMES_time, offsetof (struct X3D_TimeSensor, time),  FIELDTYPE_SFTime, KW_outputOnly,
		        FIELDNAMES___inittime, offsetof (struct X3D_TimeSensor, __inittime),  FIELDTYPE_SFTime, KW_initializeOnly,
		        FIELDNAMES_fraction_changed, offsetof (struct X3D_TimeSensor, fraction_changed),  FIELDTYPE_SFFloat, KW_outputOnly,
		        FIELDNAMES_loop, offsetof (struct X3D_TimeSensor, loop),  FIELDTYPE_SFBool, KW_inputOutput,
		        FIELDNAMES_resumeTime, offsetof (struct X3D_TimeSensor, resumeTime),  FIELDTYPE_SFTime, KW_inputOutput,
        		...
        		-1, -1, -1, -1};
		*/

		/* go til we find the field, or a -1 */
		while ((*np != -1) && (*np != fromFieldInt)) np += 4;
		np++; fromOffset = *np; np++; fromVRMLtype = *np; np++;

		/* is this a valid type? */
		if ((*np != KW_outputOnly) && (*np != KW_inputOutput)) {
			printf ("expected an output type for field %s of %s\n",
                                fieldTemp, stringNodeType(fromNode->_nodeType));
              	  	rv = FALSE;
		}

		/* get the to field info */
		np = (int *)NODE_OFFSETS[toNode->_nodeType];

		while ((*np != -1) && (*np != toFieldInt)) np += 4;
		np++; toOffset = *np; np++; toVRMLtype = *np; np++;

		/* is this a valid type? */
		if ((*np != KW_inputOnly) && (*np != KW_inputOutput)) {
			printf ("expected an input type for field %s of %s\n",
                                fieldTemp, stringNodeType(toNode->_nodeType));
              	  	rv = FALSE;
		}

		/* do the VRML types match? */
		/* printf ("VRML types %d %d\n",fromVRMLtype, toVRMLtype); */
		if (fromVRMLtype != toVRMLtype) {
			printf ("Routing type mismatch\n");
			rv = FALSE;
		}
	}

	/* ------- if we are ok, call the routing code  -------- */
	/* still ok? */
	if (rv == TRUE) {
		if (command == ADDROUTE) adrem = 1;
		else adrem = 0;

		sprintf (fieldTemp,"%d:%d",toNode,toOffset);
		CRoutes_Register(adrem, (void *)fromNode, fromOffset, 1,
			fieldTemp, returnRoutingElementLength(fromVRMLtype),
			returnInterpolatorPointer (stringNodeType(toNode->_nodeType)),
			0,0);


		strcat (buf, "0");
	} else {
		strcat (buf, "1");
	}
}


/* for a GetFieldTypes command for a node, we return a string giving the field types */

void makeFIELDDEFret(uintptr_t myptr, char *buf, int repno) {
	struct X3D_Node *boxptr;
	int myc;
	int *np;
	char myline[200];

	boxptr = X3D_NODE(myptr);
	
	/* printf ("GETFIELDDEFS, node %d\n",boxptr); */

	if (boxptr == 0) {
		printf ("makeFIELDDEFret have null node here \n");
		sprintf (buf,"RE\n%f\n%d\n0",TickTime,repno);
		return;
	}

	/* printf ("node type is %s\n",stringNodeType(boxptr->_nodeType)); */
	


	/* how many fields in this node? */
	np = (int *)NODE_OFFSETS[boxptr->_nodeType];
	myc = 0;
	while (*np != -1) {
		/* is this a hidden field? */
		if (strcmp (FIELDNAMES[*np],"_") != 0) {
			myc ++; 
		}

		np +=4;

	}

	sprintf (buf,"RE\n%f\n%d\n",TickTime,repno);

	sprintf (myline, "%d ",myc);
	strcat (buf, myline);

	/* now go through and get the name, type, keyword */
	np = (int *)NODE_OFFSETS[boxptr->_nodeType];
	while (*np != -1) {
		if (strcmp (FIELDNAMES[*np],"_") != 0) {
			sprintf (myline,"%s %c %s ",FIELDNAMES[np[0]], (char) mapFieldTypeToEAItype(np[2]), 
				KEYWORDS[np[3]]);
			strcat (buf, myline);
		}
		np += 4;
	}
}



/* EAI, replaceWorld. */
void EAI_RW(char *str) {
	struct X3D_Node *newNode;
	int i;

	/* clean the slate! keep EAI running, though */
	kill_oldWorld(FALSE,TRUE,FALSE,__FILE__,__LINE__);

	/* go through the string, and send the nodes into the rootnode */
	/* first, remove the command, and get to the beginning of node */
	while ((*str != ' ') && (strlen(str) > 0)) str++;
	while (isspace(*str)) str++;
	while (strlen(str) > 0) {
		i = sscanf (str, "%u",&newNode);
		if (i>0) AddRemoveChildren (rootNode,rootNode + offsetof (struct X3D_Group, children),&newNode,1,1);
		while (isdigit(*str)) str++;
		while (isspace(*str)) str++;
	}
}


void createLoadURL(char *bufptr) {
	#define strbrk " :loadURLStringBreak:"
	int count;
	char *spbrk;
	int retint;		/* used to get retval from sscanf */


	/* fill in Anchor parameters */
	EAI_AnchorNode.description = newASCIIString("From EAI");

	/* fill in length fields from string */
	while (*bufptr==' ') bufptr++;
	retint=sscanf (bufptr,"%d",&EAI_AnchorNode.url.n);
	while (*bufptr>' ') bufptr++;
	while (*bufptr==' ') bufptr++;
	retint=sscanf (bufptr,"%d",&EAI_AnchorNode.parameter.n);
	while (*bufptr>' ') bufptr++;
	while (*bufptr==' ') bufptr++;

	/* now, we should be at the strings. */
	bufptr--;

	/* MALLOC the sizes required */
	if (EAI_AnchorNode.url.n > 0) EAI_AnchorNode.url.p = MALLOC(EAI_AnchorNode.url.n * sizeof (struct Uni_String));
	if (EAI_AnchorNode.parameter.n > 0) EAI_AnchorNode.parameter.p = MALLOC(EAI_AnchorNode.parameter.n * sizeof (struct Uni_String));

	for (count=0; count<EAI_AnchorNode.url.n; count++) {
		bufptr += strlen(strbrk);
		/* printf ("scanning, at :%s:\n",bufptr); */
		
		/* nullify the next "strbrk" */
		spbrk = strstr(bufptr,strbrk);
		if (spbrk!=NULL) *spbrk='\0';

		EAI_AnchorNode.url.p[count] = newASCIIString(bufptr);

		if (spbrk!=NULL) bufptr = spbrk;
	}
	for (count=0; count<EAI_AnchorNode.parameter.n; count++) {
		bufptr += strlen(strbrk);
		/* printf ("scanning, at :%s:\n",bufptr); */
		
		/* nullify the next "strbrk" */
		spbrk = strstr(bufptr,strbrk);
		if (spbrk!=NULL) *spbrk='\0';

		EAI_AnchorNode.parameter.p[count] = newASCIIString(bufptr);

		if (spbrk!=NULL) bufptr = spbrk;
	}
	EAI_AnchorNode.__parenturl = newASCIIString("./");
}



/* if we have a LOADURL command (loadURL in java-speak) we call Anchor code to do this.
   here we tell the EAI code of the success/fail of an anchor call, IF the EAI is 
   expecting such a call */

void EAI_Anchor_Response (int resp) {
	char myline[1000];
	if (waiting_for_anchor) {
		if (resp) strcpy (myline,"OK\nRE_EOT");
		else strcpy (myline,"FAIL\nRE_EOT");
		EAI_send_string (myline,EAIlistenfd);
	}
	waiting_for_anchor = FALSE;
}
