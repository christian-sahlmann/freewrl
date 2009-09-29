/*
=INSERT_TEMPLATE_HERE=

$Id: Component_KeyDevice.h,v 1.1 2009/09/29 17:22:07 istakenv Exp $

Proximity sensor macro.

*/

#ifndef __FREEWRL_SCENEGRAPH_KEYDEVICE_H__
#define __FREEWRL_SCENEGRAPH_KEYDEVICE_H__

/* required def's for this file */
#include "../vrml_parser/Structs.h"	/* for X3D_Node,X3D_KeySensor,X3D_StringSensor */


/* some function def's (are these global??) */
#define X3D_KEYSENSOR(node) ((struct X3D_KeySensor*)node)
#define X3D_STRINGSENSOR(node) ((struct X3D_StringSensor*)node)


/* function protos */
static void incrementKeySinkList();
int KeySensorNodePresent();
void addNodeToKeySensorList(struct X3D_Node* node);
void killKeySensorNodeList();
void sendKeyToKeySensor(const char key, int upDown);
static void sendToKS(struct X3D_Node* wsk, int key, int upDown);
static void sendToSS(struct X3D_Node *wsk, int key, int upDown);

#endif /* __FREEWRL_SCENEGRAPH_KEYDEVICE_H__ */
