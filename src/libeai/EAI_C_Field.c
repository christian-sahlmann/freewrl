
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

#ifndef REWIRE
#include "config.h"
#include "system.h"
#endif
#include "EAI_C.h"

void X3D_freeEventIn(X3DEventIn* ev) {
	if (ev != NULL) 
		free(ev);
}

void X3D_freeEventOut(X3DEventOut* ev) {
	if (ev != NULL)
		free(ev);
} 

void X3D_freeNode(X3DNode* node) {
	int i;

	if (node == NULL) {
		printf("TRYING TO FREE NULL PTR\n");
		return;
	}

	switch (node->type) {
		case FIELDTYPE_SFVec3f:
		case FIELDTYPE_SFVec2f:
		case FIELDTYPE_SFColor:
		case FIELDTYPE_SFFloat:
		case FIELDTYPE_SFRotation:
		case FIELDTYPE_SFColorRGBA:
		case FIELDTYPE_SFBool:
		case FIELDTYPE_SFInt32:
		case FIELDTYPE_SFTime:
			free(node);
		break;

		case FIELDTYPE_SFString:
			free(node->X3D_SFString.strptr);
			free(node);

		break;

		case FIELDTYPE_MFString:
			for (i = 0; i < node->X3D_MFString.n; i++) {
				free(node->X3D_MFString.p[i].strptr);
			}
			free(node->X3D_MFString.p);
			free(node);
		break;

		case FIELDTYPE_MFFloat:
		case FIELDTYPE_MFVec3f:
		case FIELDTYPE_MFVec3d:
		case FIELDTYPE_MFVec2f:
		case FIELDTYPE_MFRotation:
		case FIELDTYPE_MFColor:
		case FIELDTYPE_MFBool:
		case FIELDTYPE_MFColorRGBA:
		case FIELDTYPE_MFInt32:
			free(node->X3D_MFInt32.p);
			free(node);
		break;
			

		default:
			free(node);

	}
}

X3DNode *X3D_newSFVec3f (float a, float b, float c) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFVec3f.type = FIELDTYPE_SFVec3f;
	retval->X3D_SFVec3f.c[0] = a;
	retval->X3D_SFVec3f.c[1] = b;
	retval->X3D_SFVec3f.c[2] = c;
	return retval;
}

void X3D_getSFVec3f(X3DNode* node, float* value) {
	if (node->X3D_SFVec3f.type != FIELDTYPE_SFVec3f) 
		return;
	value[0] = node->X3D_SFVec3f.c[0];
	value[1] = node->X3D_SFVec3f.c[1];
	value[2] = node->X3D_SFVec3f.c[2];

}

X3DNode *X3D_newSFColor (float a, float b, float c) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFColor.type = FIELDTYPE_SFColor;
	retval->X3D_SFColor.c[0] = a;
	retval->X3D_SFColor.c[1] = b;
	retval->X3D_SFColor.c[2] = c;
	return retval;
}

void  X3D_getSFColor(X3DNode* node, float* value) {
	if (node->X3D_SFColor.type != FIELDTYPE_SFColor) 
		return;
	value[0] = node->X3D_SFColor.c[0];	
	value[1] = node->X3D_SFColor.c[1];	
	value[2] = node->X3D_SFColor.c[2];	
}

X3DNode *X3D_newSFVec2f (float a, float b) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFVec2f.type = FIELDTYPE_SFVec2f;
	retval->X3D_SFVec2f.c[0] = a;
	retval->X3D_SFVec2f.c[1] = b;
	return retval;
}

void X3D_getSFVec2f(X3DNode* node, float* value) {
	if (node->X3D_SFVec2f.type != FIELDTYPE_SFVec2f) 
		return;
	value[0] = node->X3D_SFVec3f.c[0];
	value[1] = node->X3D_SFVec3f.c[1];
}

X3DNode *X3D_newSFRotation (float a, float b,float c, float d) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFRotation.type = FIELDTYPE_SFRotation;
	retval->X3D_SFRotation.r[0] = a;
	retval->X3D_SFRotation.r[1] = b;
	retval->X3D_SFRotation.r[2] = c;
	retval->X3D_SFRotation.r[3] = d;
	return retval;
}

void X3D_getSFRotation(X3DNode* node, float* value) {
	if (node->X3D_SFRotation.type != FIELDTYPE_SFRotation) 
		return;
	value[0] = node->X3D_SFRotation.r[0];
	value[1] = node->X3D_SFRotation.r[1];
	value[2] = node->X3D_SFRotation.r[2];
	value[3] = node->X3D_SFRotation.r[3];
}

X3DNode *X3D_newSFColorRGBA (float a, float b,float c, float d) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFColorRGBA.type = FIELDTYPE_SFColorRGBA;
	retval->X3D_SFColorRGBA.r[0] = a;
	retval->X3D_SFColorRGBA.r[1] = b;
	retval->X3D_SFColorRGBA.r[2] = c;
	retval->X3D_SFColorRGBA.r[3] = d;
	return retval;
}

void X3D_getSFColorRGBA(X3DNode* node, float* value) {
	if (node->X3D_SFColorRGBA.type != FIELDTYPE_SFColorRGBA)
		return;
	value[0] = node->X3D_SFColorRGBA.r[0];
	value[1] = node->X3D_SFColorRGBA.r[1];
	value[2] = node->X3D_SFColorRGBA.r[2];
	value[3] = node->X3D_SFColorRGBA.r[3];
}

X3DNode *X3D_newSFBool (int a) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFBool.type = FIELDTYPE_SFBool;
	retval->X3D_SFBool.value = a;
	return retval;
}

void X3D_getSFBool(X3DNode* node, int* value) {
	if (node->X3D_SFBool.type != FIELDTYPE_SFBool)
		return;
	*value = node->X3D_SFBool.value;
}

X3DNode *X3D_newSFFloat (float a) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFFloat.type = FIELDTYPE_SFFloat;
	retval->X3D_SFFloat.value = a;
	return retval;
}

void X3D_getSFFloat(X3DNode* node, float* value) {
	if (node->X3D_SFFloat.type != FIELDTYPE_SFFloat)
		return;
	*value = node->X3D_SFFloat.value;
}

X3DNode *X3D_newSFTime (double a) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFTime.type = FIELDTYPE_SFTime;
	retval->X3D_SFTime.value = a;
	return retval;
}

void X3D_getSFTime(X3DNode* node, double* value) {
	if (node->X3D_SFTime.type != FIELDTYPE_SFTime) 
		return;
	*value = node->X3D_SFTime.value;
}

X3DNode *X3D_newSFInt32 (int a) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFInt32.type = FIELDTYPE_SFInt32;
	retval->X3D_SFInt32.value = a;
	return retval;
}

void X3D_getSFInt32(X3DNode* node, int* value) {
	if (node->X3D_SFInt32.type != FIELDTYPE_SFInt32)
		return;
	*value = node->X3D_SFBool.value;
}

X3DNode *X3D_newSFString(char* string) {
	X3DNode *retval;
	retval = malloc (sizeof(X3DNode));
	retval->X3D_SFString.type = FIELDTYPE_SFString;
	retval->X3D_SFString.len = strlen(string);
	retval->X3D_SFString.strptr = malloc((strlen(string) + 1)*sizeof(char));
	strncpy(retval->X3D_SFString.strptr, string, strlen(string));
	retval->X3D_SFString.strptr[strlen(string)] = '\0';
	return retval;
}

char* X3D_getSFString(X3DNode* node) {
	char* string;
	if (node->type != FIELDTYPE_SFString)
		return 0;
	string = malloc((node->X3D_SFString.len + 1)*sizeof(char));
	strncpy(string, node->X3D_SFString.strptr, node->X3D_SFString.len);
	string[node->X3D_SFString.len] = '\0';
	return string;
}

X3DNode *X3D_newMFInt32(int num, int* array){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFInt32;
	retval->X3D_MFInt32.n = num;
	retval->X3D_MFInt32.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFInt32.p[i].value = array[i];		
		retval->X3D_MFInt32.p[i].type= FIELDTYPE_SFInt32;		
	}

	return retval;
}

void X3D_getMFInt32(X3DNode* node, int** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFInt32)
		return;
	*num = node->X3D_MFInt32.n;

	*array = malloc (node->X3D_MFInt32.n * sizeof(int));

	for (i = 0; i < node->X3D_MFInt32.n; i++) {
		(*array)[i] = node->X3D_MFInt32.p[i].value;
	}
}

X3DNode *X3D_newMFFloat(int num, float* array){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFFloat;
	retval->X3D_MFFloat.n = num;
	retval->X3D_MFFloat.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFFloat.p[i].value = array[i];		
		retval->X3D_MFFloat.p[i].type= FIELDTYPE_SFFloat;		
	}

	return retval;
}

void X3D_getMFFloat(X3DNode* node, float** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFFloat)
		return;
	*num = node->X3D_MFFloat.n;

	*array = malloc (node->X3D_MFFloat.n * sizeof(float));

	for (i = 0; i < node->X3D_MFFloat.n; i++) {
		(*array)[i] = node->X3D_MFFloat.p[i].value;
	}
}

X3DNode *X3D_newMFVec3f(int num, float(* array)[3]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFVec3f;
	retval->X3D_MFVec3f.n = num;
	retval->X3D_MFVec3f.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFVec3f.p[i].type= FIELDTYPE_SFVec3f;		
		retval->X3D_MFVec3f.p[i].c[0]= array[i][0];		
		retval->X3D_MFVec3f.p[i].c[1]= array[i][1];		
		retval->X3D_MFVec3f.p[i].c[2]= array[i][2];		
	}

	return retval;
}

void X3D_getMFVec3f(X3DNode* node, float*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFVec3f)
		return;
	*num = node->X3D_MFVec3f.n;

	(*array) = (float**) malloc(node->X3D_MFVec3f.n*sizeof(float*));
	(*array)[0] = (float*) malloc (node->X3D_MFVec3f.n * sizeof(float) * 3);
	for (i = 0; i < node->X3D_MFVec2f.n; i++) 
		(*array)[i] = (*array)[0] + i * 3;

	for (i = 0; i < node->X3D_MFFloat.n; i++) {
		(*array)[i][0] = node->X3D_MFVec3f.p[i].c[0];
		(*array)[i][1] = node->X3D_MFVec3f.p[i].c[1];
		(*array)[i][2] = node->X3D_MFVec3f.p[i].c[2];
	}
}

void X3D_getMFColor(X3DNode* node, float*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFColor)
		return;
	*num = node->X3D_MFColor.n;

	(*array) = (float**) malloc(node->X3D_MFColor.n*sizeof(float*));
	(*array)[0] = (float*) malloc (node->X3D_MFColor.n * sizeof(float) * 3);
	for (i = 0; i < node->X3D_MFFloat.n; i++) 
		(*array)[i] = (*array)[0] + i * 3;

	for (i = 0; i < node->X3D_MFFloat.n; i++) {
		(*array)[i][0] = node->X3D_MFColor.p[i].c[0];
		(*array)[i][1] = node->X3D_MFColor.p[i].c[1];
		(*array)[i][2] = node->X3D_MFColor.p[i].c[2];
	}
}

X3DNode *X3D_newMFColor(int num, float(* array)[3]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFColor;
	retval->X3D_MFColor.n = num;
	retval->X3D_MFColor.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFColor.p[i].type= FIELDTYPE_SFColor;		
		retval->X3D_MFColor.p[i].c[0]= array[i][0];		
		retval->X3D_MFColor.p[i].c[1]= array[i][1];		
		retval->X3D_MFColor.p[i].c[2]= array[i][2];		
	}

	return retval;
}

X3DNode *X3D_newMFVec2f(int num, float(* array)[2]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFVec2f;
	retval->X3D_MFVec2f.n = num;
	retval->X3D_MFVec2f.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFVec2f.p[i].type= FIELDTYPE_SFVec2f;
		retval->X3D_MFVec2f.p[i].c[0]= array[i][0];		
		retval->X3D_MFVec2f.p[i].c[1]= array[i][1];		
	}

	return retval;
}

void X3D_getMFVec2f(X3DNode* node, float*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFVec2f)
		return;
	*num = node->X3D_MFVec2f.n;

	(*array) = (float**) malloc(node->X3D_MFVec2f.n*sizeof(float*));
	(*array)[0] = (float*) malloc (node->X3D_MFVec2f.n * sizeof(float) * 2);
	for (i = 0; i < node->X3D_MFVec2f.n; i++) 
		(*array)[i] = (*array)[0] + i * 2;

	for (i = 0; i < node->X3D_MFVec2f.n; i++) {
		(*array)[i][0] = node->X3D_MFVec2f.p[i].c[0];
		(*array)[i][1] = node->X3D_MFVec2f.p[i].c[1];
	}
}

X3DNode *X3D_newMFRotation(int num, float(* array)[4]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFRotation;
	retval->X3D_MFRotation.n = num;
	retval->X3D_MFRotation.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFRotation.p[i].type= FIELDTYPE_SFRotation;		
		retval->X3D_MFRotation.p[i].r[0]= array[i][0];		
		retval->X3D_MFRotation.p[i].r[1]= array[i][1];		
		retval->X3D_MFRotation.p[i].r[2]= array[i][2];		
		retval->X3D_MFRotation.p[i].r[3]= array[i][3];		
	}

	return retval;
}

void X3D_getMFRotation(X3DNode* node, float*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFRotation)
		return;
	*num = node->X3D_MFRotation.n;

	(*array) = (float**) malloc(node->X3D_MFRotation.n*sizeof(float*));
	(*array)[0] = (float*) malloc (node->X3D_MFRotation.n * sizeof(float) * 4);
	for (i = 0; i < node->X3D_MFRotation.n; i++) 
		(*array)[i] = (*array)[0] + i * 4;

	for (i = 0; i < node->X3D_MFFloat.n; i++) {
		(*array)[i][0] = node->X3D_MFRotation.p[i].r[0];
		(*array)[i][1] = node->X3D_MFRotation.p[i].r[1];
		(*array)[i][2] = node->X3D_MFRotation.p[i].r[2];
		(*array)[i][3] = node->X3D_MFRotation.p[i].r[3];
	}
}

X3DNode *X3D_newMFColorRGBA(int num, float(* array)[4]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFColorRGBA;
	retval->X3D_MFColorRGBA.n = num;
	retval->X3D_MFColorRGBA.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFColorRGBA.p[i].type= FIELDTYPE_SFColorRGBA;
		retval->X3D_MFColorRGBA.p[i].r[0]= array[i][0];		
		retval->X3D_MFColorRGBA.p[i].r[1]= array[i][1];		
		retval->X3D_MFColorRGBA.p[i].r[2]= array[i][2];		
		retval->X3D_MFColorRGBA.p[i].r[3]= array[i][3];		
	}

	return retval;
}

void X3D_getMFColorRGBA(X3DNode* node, float*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFColorRGBA)
		return;
	*num = node->X3D_MFColorRGBA.n;

	(*array) = (float**) malloc(node->X3D_MFColorRGBA.n*sizeof(float*));
	(*array)[0] = (float*) malloc (node->X3D_MFColorRGBA.n * sizeof(float) * 4);
	for (i = 0; i < node->X3D_MFColorRGBA.n; i++) 
		(*array)[i] = (*array)[0] + i * 4;

	for (i = 0; i < node->X3D_MFColorRGBA.n; i++) {
		(*array)[i][0] = node->X3D_MFColorRGBA.p[i].r[0];
		(*array)[i][1] = node->X3D_MFColorRGBA.p[i].r[1];
		(*array)[i][2] = node->X3D_MFColorRGBA.p[i].r[2];
		(*array)[i][3] = node->X3D_MFColorRGBA.p[i].r[3];
	}
}

X3DNode *X3D_newMFBool(int num, int* array){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFBool;
	retval->X3D_MFBool.n = num;
	retval->X3D_MFBool.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFBool.p[i].value = array[i];		
		retval->X3D_MFBool.p[i].type= FIELDTYPE_SFBool;		
	}

	return retval;
}

void X3D_getMFBool(X3DNode* node, int** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFBool)
		return;
	*num = node->X3D_MFBool.n;

	*array = malloc (node->X3D_MFBool.n * sizeof(int));

	for (i = 0; i < node->X3D_MFBool.n; i++) {
		(*array)[i] = node->X3D_MFBool.p[i].value;
	}
}

X3DNode *X3D_newMFVec3d(int num, double(* array)[3]){
	int i;
	X3DNode* retval;

	retval = malloc(sizeof(X3DNode));
	retval->type = FIELDTYPE_MFVec3d;
	retval->X3D_MFVec3d.n = num;
	retval->X3D_MFVec3d.p = malloc (sizeof(X3DNode) * num);

	for (i = 0; i < num; i++) {
		retval->X3D_MFVec3d.p[i].type= FIELDTYPE_SFVec3d;		
		retval->X3D_MFVec3d.p[i].c[0]= array[i][0];		
		retval->X3D_MFVec3d.p[i].c[1]= array[i][1];		
		retval->X3D_MFVec3d.p[i].c[2]= array[i][2];		
	}

	return retval;
}

void X3D_getMFVec3d(X3DNode* node, double*** array, int* num) {
	int i;

	if (node->type != FIELDTYPE_MFVec3d)
		return;
	*num = node->X3D_MFVec3d.n;

	(*array) = (double**) malloc(node->X3D_MFVec3d.n*sizeof(double*));
	(*array)[0] = (double*) malloc (node->X3D_MFVec3d.n * sizeof(double) * 3);
	for (i = 0; i < node->X3D_MFVec3d.n; i++)  {
		(*array)[i] = (*array)[0] + i * 3;
		(*array)[i][1] = node->X3D_MFVec3d.p[i].c[1];
		(*array)[i][2] = node->X3D_MFVec3d.p[i].c[2];
	}
}

X3DNode *X3D_newMFString(int num, char array[][STRLEN]){
	int i;
	X3DNode* retval;
	retval = malloc (sizeof (X3DNode));
	retval->type = FIELDTYPE_MFString;
	retval->X3D_MFString.n = num;
	retval->X3D_MFString.p = malloc (sizeof (X3DNode) * num);

	for (i = 0; i < num; i++) {
#ifndef OLDCODE
		/* Doug Sanden changes */
		retval->X3D_MFString.p[i].type = FIELDTYPE_SFString; /*based on pattern above ie vec3f this should be SF */
#else
		retval->X3D_MFString.p[i].type = FIELDTYPE_MFString;
#endif
		retval->X3D_MFString.p[i].len = strlen(array[i]);
		retval->X3D_MFString.p[i].strptr = malloc(sizeof(char)*STRLEN);
		strncpy(retval->X3D_MFString.p[i].strptr, array[i], STRLEN);
	}

	return retval;
}

void X3D_getMFString(X3DNode* node, char*** array, int* num) {
	int i;
	
	if (node->type != FIELDTYPE_MFString)
		return;


	*num = node->X3D_MFString.n;

	(*array) = (char**) malloc(node->X3D_MFString.n*sizeof(char*));
	(*array)[0] = (char*) malloc(node->X3D_MFString.n * sizeof(char) * STRLEN);

	for (i = 0; i < node->X3D_MFString.n; i++) {
		(*array)[i] = (*array)[0] + (i*256);
		strncpy((*array)[i], node->X3D_MFString.p[i].strptr, STRLEN);
	}
}

X3DNode *X3D_newSFNode(){printf ("New node not implemented yet for this type\n");return NULL;}
X3DNode *X3D_newSFImage(){printf ("New node not implemented yet for this type\n");return NULL;}
X3DNode *X3D_newMFNode(){printf ("New node not implemented yet for this type\n");return NULL;}
/* Nodes not used in FreeWRL */
X3DNode *X3D_newMFVec2d(int num){printf ("New node not implemented yet for this type\n");return NULL;}
X3DNode *X3D_newMFTime(int num){printf ("New node not implemented yet for this type\n");return NULL;}
X3DNode *X3D_newSFVec2d (double a, double b){printf ("New node not implemented yet for this type\n");return NULL;}
X3DNode *X3D_newSFVec3d (double a, double b,double c){printf ("New node not implemented yet for this type\n");return NULL;}
char *fieldTypeName(char type){printf ("New node not implemented yet for this type\n");return NULL;}


