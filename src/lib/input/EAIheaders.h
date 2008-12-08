/*
=INSERT_TEMPLATE_HERE=

$Id: EAIheaders.h,v 1.3 2008/12/08 17:58:48 crc_canada Exp $

EAI and java CLASS invocation

*/

#ifndef __FREEX3D_EAI_H__
#define __FREEX3D_EAI_H__


int eaiverbose;

/* function prototypes */
void handle_Listener (void);
void EAI_Convert_mem_to_ASCII (int id, char *reptype, int type, char *memptr, char *buf);
unsigned int EAI_SendEvent (char *ptr);
void EAI_RNewW(char *bufptr);
void EAI_RW(char *bufptr);


#define MAXEAIHOSTNAME	255		/* length of hostname on command line */
#define EAIREADSIZE	8192		/* maximum we are allowed to read in from socket */
#define EAIBASESOCKET   9877		/* socket number to start at */


/* these are commands accepted from the EAI client */
#define GETNODE		'A'
#define GETEAINODETYPE	'B'
#define SENDCHILD 	'C'
#define SENDEVENT	'D'
#define GETVALUE	'E'
#define GETFIELDTYPE	'F'
#define	REGLISTENER	'G'
#define	ADDROUTE	'H'
#define REREADWRL	'I'
#define	DELETEROUTE	'J'
#define GETNAME		'K'
#define	GETVERSION	'L'
#define GETCURSPEED	'M'
#define GETFRAMERATE	'N'
#define	GETURL		'O'
#define	REPLACEWORLD	'P'
#define	LOADURL		'Q'
#define VIEWPOINT	'R'
#define CREATEVS	'S'
#define	CREATEVU	'T'
#define	STOPFREEWRL	'U'
#define UNREGLISTENER   'W'
#define GETRENDPROP	'X'
#define GETENCODING	'Y'
#define CREATENODE	'a'
#define CREATEPROTO	'b'
#define UPDNAMEDNODE	'c'
#define REMNAMEDNODE	'd'
#define GETPROTODECL 	'e'
#define UPDPROTODECL	'f'
#define REMPROTODECL	'g'
#define GETFIELDDEFS	'h'
#define GETNODEDEFNAME	'i'
#define GETROUTES	'j'
#define GETNODETYPE	'k'
#define MIDIINFO  	'l'
#define MIDICONTROL  	'm'


/* command string to get the rootNode - this is a special match... */
#define SYSTEMROOTNODE "_Sarah_this_is_the_FreeWRL_System_Root_Node"


/* Subtypes - types of data to get from EAI  - we don't use the ones defined in
   headers.h, because we want ASCII characters */

#define	EAI_SFBool		'b'
#define	EAI_SFColor		'c'
#define	EAI_SFFloat		'd'
#define	EAI_SFTime		'e'
#define	EAI_SFInt32		'f'
#define	EAI_SFString		'g'
#define	EAI_SFNode		'h'
#define	EAI_SFRotation		'i'
#define	EAI_SFVec2f		'j'
#define	EAI_SFImage		'k'
#define	EAI_MFColor		'l'
#define	EAI_MFFloat		'm'
#define	EAI_MFTime		'n'
#define	EAI_MFInt32		'o'
#define	EAI_MFString		'p'
#define	EAI_MFNode		'q'
#define	EAI_MFRotation		'r'
#define	EAI_MFVec2f		's'
#define EAI_MFVec3f		't'
#define EAI_SFVec3f		'u'
#define EAI_MFColorRGBA		'v'
#define EAI_SFColorRGBA		'w'
#define EAI_MFBool		'x'
#define EAI_FreeWRLPTR		'y'
#define EAI_MFVec3d		'A'
#define EAI_SFVec2d		'B'
#define EAI_SFVec3d		'C'
#define EAI_MFVec2d		'D'
#define EAI_SFVec4d		'E'
#define EAI_MFDouble		'F'
#define EAI_SFDouble		'G'
#define EAI_SFMatrix3f		'H'
#define EAI_MFMatrix3f		'I'
#define EAI_SFMatrix3d		'J'
#define EAI_MFMatrix3d		'K'
#define EAI_SFMatrix4f		'L'
#define EAI_MFMatrix4f		'M'
#define EAI_SFMatrix4d		'N'
#define EAI_MFMatrix4d		'O'
#define EAI_SFVec4f		'P'
#define EAI_MFVec4f		'Q'
#define EAI_MFVec4d		'R'



/* Function Prototype for plugins, Java Class Invocation */
int createUDPSocket();
int conEAIorCLASS(int socketincrement, int *sockfd, int *listenfd);
void EAI_send_string (char *str, int listenfd);
char *read_EAI_socket(char *bf, int *bfct, int *bfsz, int *listenfd);
extern int EAIlistenfd;
extern int EAIsockfd;
extern int EAIport;
extern int EAIwanted;
extern int EAIbufsize;
extern char *EAIbuffer;
extern int EAIbufcount;
extern char EAIListenerData[EAIREADSIZE];
extern char EAIListenerArea[40];

#define MIDI_CONTROLLER_UNUSED 4
#define MIDI_CONTROLLER_FADER 1
#define MIDI_CONTROLLER_KEYPRESS 2
#define MIDI_CONTROLLER_UNKNOWN 999


#endif /* __FREEX3D_EAI_H__ */
