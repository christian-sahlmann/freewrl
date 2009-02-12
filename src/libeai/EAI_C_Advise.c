#include "config.h"
#include "system.h"
#include "EAI_C.h"

#define LOCK_ADVISE_TABLE printf ("locking advise table\n");
#define UNLOCK_ADVISE_TABLE printf ("unlocking advise table\n");

struct EAI_ListenerStruct {
	int FreeWRL_RegisterNumber;
	int type;
	void *dataArea;
	void    (*functionHandler)(void *);
};

struct EAI_ListenerStruct *EAI_ListenerTable = 0;
int MaxEAIListeners = 0;
int AdviseIndex = -1;

int X3DAdvise (X3DEventOut *node, void *fn) {

	AdviseIndex ++;
	/* Browser.RegisterListener (f, userData, nodeptr,offset,datatype , datasize, EventType); */

	/* save the data, and the node, so that if this listener is called, we can call
		the function and pass it the correct X3DNode */

	/*printf ("in X3DAdvise, we have queryno %d nodeptr %d offset %d datatype %d datasize %d field %s\n",
		AdviseIndex, node->nodeptr, node->offset, node->datatype, node->datasize, node->field); */

/*
 EAIoutSender.send ("" + queryno + "G " + nodeptr + " " + offset + " " + datatype +
                " " + datasize + "\n"); 
*/




	if (AdviseIndex >= MaxEAIListeners) {
		/* oooh! not enough room at the table */
		LOCK_ADVISE_TABLE
		MaxEAIListeners += 100; /* arbitrary number */
		EAI_ListenerTable = (struct EAI_ListenerStruct*)realloc (EAI_ListenerTable, sizeof(*EAI_ListenerTable) * MaxEAIListeners);
		UNLOCK_ADVISE_TABLE
	}

	/* record this one... */
	EAI_ListenerTable[AdviseIndex].type = node->datatype;
	EAI_ListenerTable[AdviseIndex].FreeWRL_RegisterNumber = _X3D_queryno;
	if (node->datasize>0)
		EAI_ListenerTable[AdviseIndex].dataArea = malloc (node->datasize);
	else 
		EAI_ListenerTable[AdviseIndex].dataArea = NULL;
	EAI_ListenerTable[AdviseIndex].functionHandler = fn;

	/* and, tell FreeWRL about this one */
	_RegisterListener (node,AdviseIndex);
	
	return TRUE;
}


void _handleFreeWRLcallback (char *line) {
	double evTime;
	int evIndex;
	int count;

	/* something funny at the beginning of time? */
	if (AdviseIndex < 0) return;

	if (!isSwig) {
	if (strstr(line,"EV_EOT") == NULL) {
		printf ("handle_callback - no eot in string %s\n",line);
	} else {
		/* skip past the "EV" and get to the event time */
		while ((!isdigit(*line)) && (*line != '\0')) line++; 
		sscanf (line, "%lf",&evTime);

		/* get the event number */
		while (!iscntrl(*line)) line++; while (iscntrl(*line)) line++;
		sscanf (line,"%d",&evIndex);

		/* get to the data */
		while (!iscntrl(*line)) line++; while (iscntrl(*line)) line++;

		#ifdef VERBOSE
		printf ("event time %lf index %d data :%s:\n",evTime, evIndex, line);
		#endif

		/* does this advise callback exist? */
		count=0;
		while (EAI_ListenerTable[count].FreeWRL_RegisterNumber != evIndex) {
			printf ("compared %d to %d\n",EAI_ListenerTable[count].FreeWRL_RegisterNumber, evIndex);
			count ++; 
			if (count > AdviseIndex) {
				printf ("hmmm - Advise retval %d >= max %d\n",count,AdviseIndex);
				return;
			}
		}

		/* ok, we have the Advise Index. */
		if (EAI_ListenerTable[count].dataArea != NULL) {
			Parser_scanStringValueToMem(EAI_ListenerTable[count].dataArea, 0,
			EAI_ListenerTable[count].type, line, 0);

		}
		EAI_ListenerTable[count].functionHandler(EAI_ListenerTable[count].dataArea);
	}
	} else {
		while ((!isdigit(*line)) && (*line != '\0')) line++; 
		sscanf (line, "%lf",&evTime);
		while (!iscntrl(*line)) line++; while (iscntrl(*line)) line++;
		write(_X3D_FreeWRL_Swig_FD, line, sizeof(line));
	}
}
