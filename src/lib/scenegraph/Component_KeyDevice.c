/*
=INSERT_TEMPLATE_HERE=

$Id: Component_KeyDevice.c,v 1.2 2008/11/27 00:27:18 couannette Exp $

X3D Key Device Component

*/

#include <config.h>
#include <system.h>
#include <display.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "../vrml_parser/Structs.h" /* point_XYZ */
#include "../main/headers.h"


/*
I'll leave off comments about the validity of this part of the spec - it does
not seem well thought out. I'll leave off my comments on what I really think
about this part of the X3D Spec, because I'm nice, and I don't want to 
put other people's ideas down, especially in public, in source code, that
will outlive me. 

So, if there is a KeyDevice node present, DO NOT use keys for FreeWRL navigation
but instead, send any along that the Operating System GUI does not capture,
and hope that they are not too badly mangled by intervening layers.

Lets just hope that this part of the spec dies a convenient (and speedy)
death!

Anyway, with that, lets blindly forge along...

*********************************************************************/

#define X3D_KEYSENSOR(node) ((struct X3D_KeySensor*)node)
#define X3D_STRINGSENSOR(node) ((struct X3D_StringSensor*)node)
static void sendToKS(struct X3D_Node * wsk, int key, int upDown);
static void sendToSS(struct X3D_Node * wsk, int key, int upDown);

#ifndef AQUA
int shiftPressed = 0;
int ctrlPressed = 0;
#endif

/* mapped from my Apple OSX keyboard, canadian setup, so here goes... */
#ifndef AQUA
#define HOME_KEY 80
#define PGDN_KEY 86
#define LEFT_KEY 106
#define END_KEY 87
#define UP_KEY 112
#define RIGHT_KEY 108
#define PGUP_KEY 85
#define DOWN_KEY 59
#define F1_KEY  0xFFBE
#define F2_KEY  0xFFBF
#define F3_KEY  0XFFC0
#define F4_KEY  0XFFC1
#define F5_KEY  0XFFC2
#define F6_KEY  0XFFC3
#define F7_KEY  0XFFC4
#define F8_KEY  0XFFC5
#define F9_KEY  0XFFC6
#define F10_KEY 0XFFC7
#define F11_KEY 0XFFC8
#define F12_KEY 0XFFC9
#define ALT_KEY	0XFFE7
#define CTL_KEY 0XFFE3
#define SFT_KEY 0XFFE1
#define DEL_KEY 0x08
#define RTN_KEY 13
#define KEYDOWN 2
#define KEYUP	3
#else
#define HOME_KEY 0x29
#define PGDN_KEY 0x2d
#define LEFT_KEY 0x02
#define END_KEY  0x3b
#define UP_KEY   0x00
#define RIGHT_KEY 0x03
#define PGUP_KEY 0x2c
#define DOWN_KEY 0x01
#define F1_KEY  0x4
#define F2_KEY  0x5
#define F3_KEY  0X6
#define F4_KEY  0X7
#define F5_KEY  0X8
#define F6_KEY  0X9
#define F7_KEY  0X10
#define F8_KEY  0X11
#define F9_KEY  0X12
#define F10_KEY 0X13
#define F11_KEY 0X14
#define F12_KEY 0X15
#define ALT_KEY	0X0 /* not available on OSX */
#define CTL_KEY 0X0 /* not available on OSX */
#define SFT_KEY 0X0 /* not available on OSX */
#define DEL_KEY 0x7F
#define RTN_KEY 13
#define KEYDOWN 2
#define KEYUP	3
#endif


/* only keep 1 keyDevice node around; we can make a list if that is eventually
required by the spec. From what I can see, the spec is silent on this regard */

static struct X3D_Node **keySink = NULL;
static int keySyncMallocLen = 0;
static int keySinkCurMax = 0;

static void incrementKeySinkList() {
	if (keySinkCurMax >= keySyncMallocLen) {
		keySyncMallocLen += 10; /* arbitrary number */
		keySink = REALLOC(keySink, sizeof (struct X3D_Node *) * keySyncMallocLen);
	}
}

int KeySensorNodePresent() {
	int count;

	/* no KeyDevice node present */
	if (keySink == NULL) return FALSE;

	for (count=0; count < keySinkCurMax; count++) {
		/* hmmm, there is one, but is it enabled? */
		/* printf ("ks, checking %d\n",keySink[count]); */

		if (keySink[count]->_nodeType == NODE_KeySensor) 
			if (X3D_KEYSENSOR(keySink[count])->enabled) return TRUE;

		if (keySink[count]->_nodeType == NODE_StringSensor) 
			if (X3D_STRINGSENSOR(keySink[count])->enabled) return TRUE;
	}

	return FALSE;
}


void addNodeToKeySensorList(struct X3D_Node* node) {
	if ((node->_nodeType == NODE_KeySensor) || (node->_nodeType == NODE_StringSensor)) {
		incrementKeySinkList();
		keySink[keySinkCurMax] = node;
		keySinkCurMax ++;
	}
}

void killKeySensorNodeList() {

	FREE_IF_NZ(keySink);
	keySyncMallocLen = 0; 
	keySinkCurMax = 0;

	#ifndef AQUA
	shiftPressed = 0;
	ctrlPressed = 0;
	#endif
}

void sendKeyToKeySensor(const char key, int upDown) {
	int count;
	if (keySink == NULL) return;

	for (count=0; count < keySinkCurMax; count++) {
		if (keySink[count]->_nodeType == NODE_KeySensor) sendToKS(keySink[count], (int)key&0xFFFF, upDown);
		if (keySink[count]->_nodeType == NODE_StringSensor) sendToSS(keySink[count], (int)key&0xFFFF, upDown);
	}
}

/*******************************************************/

static void sendToKS(struct X3D_Node* wsk, int key, int upDown) {
	#define MYN X3D_KEYSENSOR(wsk)
	/* printf ("sending key %x %u upDown %d to keySenors\n",key,key,upDown); */
	
	/* if not enabled, do nothing */
	if (!MYN) return;
	if (MYN->__oldEnabled != MYN->enabled) {
		MYN->__oldEnabled = MYN->enabled;
		MARK_EVENT(X3D_NODE(MYN),offsetof (struct X3D_KeySensor, enabled));
	}
	if (!MYN->enabled) return;

	/* is this an ACTION (tm) key  press or release? */
	switch (key) {
		case HOME_KEY:
		case PGDN_KEY:
		case LEFT_KEY:
		case END_KEY:
		case UP_KEY:
		case RIGHT_KEY:
		case PGUP_KEY:
		case DOWN_KEY:
		case F1_KEY:
		case F2_KEY:
		case F3_KEY:
		case F4_KEY:
		case F5_KEY:
		case F6_KEY:
		case F7_KEY:
		case F8_KEY:
		case F9_KEY:
		case F10_KEY:
		case F11_KEY:
		case F12_KEY:
			if (upDown == KEYDOWN)  {
				MYN->actionKeyPress = TRUE; 
				MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, actionKeyPress));
			} else {
				MYN->actionKeyRelease = TRUE;
				MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, actionKeyRelease));
			}
			break;
		default: {
			if ((MYN->keyPress->len != 2) || (MYN->keyRelease->len != 2)) {
				FREE_IF_NZ(MYN->keyPress->strptr);
				FREE_IF_NZ(MYN->keyRelease->strptr);
				MYN->keyPress = newASCIIString ("a");
				MYN->keyRelease = newASCIIString ("a");
			}
				
			if (upDown == KEYDOWN) {
				MYN->keyPress->strptr[0] = (char) (key&0xFF);
				MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, keyPress));
			} else {
				MYN->keyRelease->strptr[0] = (char) (key&0xFF);
				MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, keyRelease));
			}
		}
	}

	/* now, for some of the other keys, the ones that are modifiers, not ACTION (tm) keys. */
	MYN->altKey = key==ALT_KEY;
	MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, altKey));
	MYN->controlKey = key==ALT_KEY;
	MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, controlKey));
	MYN->shiftKey = key==ALT_KEY;
	MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, shiftKey));

	/* now, presumably "isActive" means that the key is down... */
	MYN->isActive = upDown == KEYDOWN;
	MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_KeySensor, isActive));
	#undef MYN
	
}
static void sendToSS(struct X3D_Node *wsk, int key, int upDown) {
	#define MYN X3D_STRINGSENSOR(wsk)
	#define MAXSTRINGLEN 512

	/* printf ("SS, %u enabled %d\n",wsk, MYN->enabled); */

	/* if not enabled, do nothing */
	if (!MYN) return;
	if (MYN->__oldEnabled != MYN->enabled) {
		MYN->__oldEnabled = MYN->enabled;
		MARK_EVENT(X3D_NODE(MYN),offsetof (struct X3D_StringSensor, enabled));
	}
	if (!MYN->enabled) return;
	/* printf ("sending key %x %u upDown %d to keySenors\n",key,key,upDown); */

	#ifndef AQUA
	/* on Unix, we have to handle control/shift keys ourselves. OSX handles this
	   by itself */
	if (key == SFT_KEY) {
		shiftPressed = (upDown == KEYDOWN);
		return;
	}

	/* ignore the control key here. OSX will not event let one come this far... */
	if (key == CTL_KEY) return;

	/* do the shift of the A-Z keys if shift pressed */
	if ((key >= 'a') && (key<='z'))
		if (shiftPressed)
			key=key-'a'+'A';
	#endif

	/* we only care about key presses here */
	if (upDown != KEYDOWN) return;


	/* is this initialized? */
	if (!MYN->_initialized) {
		FREE_IF_NZ(MYN->enteredText->strptr);
		FREE_IF_NZ(MYN->finalText->strptr);
		MYN->enteredText->strptr = MALLOC(MAXSTRINGLEN+1);
		MYN->finalText->strptr = MALLOC(MAXSTRINGLEN+1);
		MYN->enteredText->len=1;
		MYN->finalText->len=1;
		MYN->enteredText->strptr[0] = '\0';
		MYN->finalText->strptr[0] = '\0';
		MYN->_initialized = TRUE;
	}
	
	/* enteredText */
	if ((MYN->deletionAllowed) && (key==DEL_KEY)) {
		if (MYN->enteredText->len > 1) {
			MYN->enteredText->len--;
			MYN->enteredText->strptr[MYN->enteredText->len-1] = '\0';
			MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, enteredText));
		}
	} else {
		if ((key != RTN_KEY) && (key != DEL_KEY) && (MYN->enteredText->len < MAXSTRINGLEN-1)) {
			MYN->enteredText->strptr[MYN->enteredText->len-1] = (char)key;
			MYN->enteredText->strptr[MYN->enteredText->len] = '\0';
			MYN->enteredText->len++;
			MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, enteredText));

			if (!MYN->isActive) {
				MYN->isActive = TRUE;
				MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, isActive));
			}
			
		}
	}


	/* printf ("enteredText:%s:\n",MYN->enteredText->strptr); */
	/* finalText */
	if (key==RTN_KEY) {
		memcpy(MYN->finalText->strptr, MYN->enteredText->strptr, MAXSTRINGLEN);
		MYN->finalText->len = MYN->enteredText->len;
		MYN->enteredText->len=1;
		MYN->enteredText->strptr[0] = '\0';
		MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, finalText));
		MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, enteredText));

		MYN->isActive = FALSE;
		MARK_EVENT(X3D_NODE(MYN), offsetof (struct X3D_StringSensor, isActive));
		/* printf ("finalText:%s:\n",MYN->finalText->strptr); */
	}
}
