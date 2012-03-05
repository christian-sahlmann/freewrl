#include <config.h>

#if !defined(IPHONE) && !defined(_ANDROID) && !defined(GLES2)

/* DJTRACK_OSCSENSORS */

typedef int (*functions)(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data) ;

int nullOSC_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data) ;
int defaultOSC_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data) ;

int nullOSC_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
	struct X3D_OSC_Sensor *realnode ;
	realnode = (struct X3D_OSC_Sensor *) user_data ;

	printf("nullOSC_handler (%s,%d) : description='%s'\n",__FILE__,__LINE__, realnode->description->strptr) ;
	printf("nullOSC_handler (%s,%d) : filter=%s\n",__FILE__,__LINE__, realnode->filter->strptr) ;
	printf("nullOSC_handler (%s,%d) : listenfor=%s\n",__FILE__,__LINE__, realnode->listenfor->strptr);

	printf("%s (%d,%s) <-", path, argc,types);
	int i ;
	for (i=0 ; i < argc ; i++) {
		switch (types[i]) {
			case 'f':
				printf(" %c:%f", types[i], argv[i]->f) ;
				break;
			case 'i':
				printf(" %c:%d", types[i], argv[i]->i) ;
				break;
			default:
				printf(" %c:??", types[i]) ;
				break;
		}
	}
	printf("\n\n") ;
	fflush(stdout);

	return 0;
}
int defaultOSC_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
	struct X3D_OSC_Sensor *realnode ;
	realnode = (struct X3D_OSC_Sensor *) user_data ;

	int willOverflow = 0;
	int freeCount;
	int iFltCount;
	int iIntCount;
	int iStrCount;
	int iBlobCount;
	int iMidiCount;
	int iOtherCount;
	/* Get the incoming counts */
	utilOSCcounts((char*) types,&iIntCount,&iFltCount,&iStrCount,&iBlobCount,&iMidiCount,&iOtherCount);

	/*
	 * If we have FIFOs operative, then use them...
	 * We need to do an atomic transaction, ie do not push
	 * any values unless there is space for all the values.
	 */
	if (realnode->FIFOsize > 0) {
		freeCount = RingBuffer_freeLen(realnode->_floatInpFIFO) ;
		if (iFltCount > freeCount) {willOverflow++;}

		freeCount = RingBuffer_freeLen(realnode->_int32InpFIFO ) ;
		if ((iIntCount+iMidiCount) > freeCount) {willOverflow++;}

		freeCount = RingBuffer_freeLen(realnode->_stringInpFIFO) ;
		if ((iStrCount+iBlobCount+iOtherCount) > freeCount) {willOverflow++;}
	}

/*
	printf("defaultOSC_handler : description='%s'\n", realnode->description->strptr) ;
	printf("defaultOSC_handler : filter=%s\n", realnode->filter->strptr) ;
	printf("defaultOSC_handler : listenfor=%s (got %s)\n", realnode->listenfor->strptr,types);
	printf("defaultOSC_handler : enabled=%d\n", realnode->enabled);
	printf("defaultOSC_handler : gotEvents=%d\n", realnode->gotEvents);
	printf("defaultOSC_handler : FIFOsize=%d\n", realnode->FIFOsize);
	printf("defaultOSC_handler : _status=%d\n", realnode->_status);
*/

/*
	printf("defaultOSC_handler int _renderFlags=%d\n", realnode->_renderFlags);
	printf("defaultOSC_handler int _hit=%d\n", realnode->_hit);
	printf("defaultOSC_handler int _change=%d\n", realnode->_change);
	printf("defaultOSC_handler int _nparents=%d\n", realnode->_nparents);
	printf("defaultOSC_handler int _nparalloc=%d\n", realnode->_nparalloc);
	printf("defaultOSC_handler int _ichange=%d\n", realnode->_ichange);
	printf("defaultOSC_handler int _nodeType=%d\n", realnode->_nodeType);
	printf("defaultOSC_handler int referenceCount=%d\n", realnode->referenceCount);
	printf("defaultOSC_handler int _defaultContainer=%d\n", realnode->_defaultContainer);
*/

/*
	printf("defaultOSC_handler struct Multi_Float _floatInpFIFO;
	printf("defaultOSC_handler struct Multi_Float _floatOutFIFO;
	printf("defaultOSC_handler struct Multi_Int32 _int32InpFIFO;
	printf("defaultOSC_handler struct Multi_Int32 _int32OutFIFO;
	printf("defaultOSC_handler struct Multi_Node _nodeInpFIFO;
	printf("defaultOSC_handler struct Multi_Node _nodeOutFIFO;
	printf("defaultOSC_handler struct Multi_String _stringInpFIFO;
	printf("defaultOSC_handler struct Multi_String _stringOutFIFO;
*/
	if (willOverflow > 0) {
		printf("defaultOSC_handler would overflow in %s,%d\n", __FILE__,__LINE__);
	} else {
                /* stringInp */
		#if TRACK_OSC_MSG
		printf("%s (%d,%s) <-", path, argc,types);
		#endif
		int i ;
		int pushBuffError = 0 ;
		for (i=0 ; i < argc ; i++) {
			switch (types[i]) {
				case 'f':
					#if TRACK_OSC_MSG
					printf(" %c:%f", types[i], argv[i]->f) ;
					#endif
					realnode->floatInp = (argv[i]->f) ;
					if (realnode->FIFOsize > 0) {
						#if TRACK_OSC_MSG
						printf("_floatInpFIFO = %p\n",realnode->_floatInpFIFO) ;
						#endif
						pushBuffError =  RingBuffer_pushFloat(realnode->_floatInpFIFO, argv[i]->f) ;
					}
					break;
				case 'i':
					#if TRACK_OSC_MSG
					printf(" %c:%d", types[i], argv[i]->i) ;
					#endif
					realnode->int32Inp = (argv[i]->i) ;
					if (realnode->FIFOsize > 0) {
						#if TRACK_OSC_MSG
						printf("_int32InpFIFO = %p\n",realnode->_int32InpFIFO) ;
						#endif
						pushBuffError =  RingBuffer_pushInt(realnode->_int32InpFIFO, argv[i]->i) ;
					}
					break;
				case 's':
					#if TRACK_OSC_MSG
					printf(" %c:%s", types[i], (char *)argv[i]) ;
					#endif
					if (realnode->stringInp != NULL) {free(realnode->stringInp);}
					realnode->stringInp = newASCIIString((char *)argv[i]);
					if (realnode->FIFOsize > 0) {
						#if TRACK_OSC_MSG
						printf("_stringInpFIFO = %p\n",realnode->_stringInpFIFO) ;
						#endif
						pushBuffError =  RingBuffer_pushPointer(realnode->_stringInpFIFO, (newASCIIString((char *)argv[i]))->strptr);
					}
					break;
				default:
					printf(" %c:??", types[i]) ;
					lo_arg_pp(types[i], argv[i]);
					break;
			}
			#if TRACK_OSC_MSG
			printf(" ");
			#endif
		}
		#if TRACK_OSC_MSG
		printf("\n\n") ;
		#endif
		fflush(stdout);

		if (realnode->enabled) {
			realnode->gotEvents += 1;
			MARK_EVENT (X3D_NODE(realnode), offsetof(struct X3D_OSC_Sensor, gotEvents));
		}
	}
	#if TRACK_OSC_MSG
	printf("\n");
	printf("defaultOSC_handler : description='%s'\n", realnode->description->strptr) ;
	printf("defaultOSC_handler : int32Inp=%d\n", realnode->int32Inp);
	printf("defaultOSC_handler : floatInp=%f\n", realnode->floatInp);
	printf("defaultOSC_handler : stringInp=%s\n", realnode->stringInp->strptr);
	printf("\n");

	if (realnode->FIFOsize > 0) {
		int qLen , iTemp ;
		float fTemp ;
		char * sTemp ;
		
		qLen = RingBuffer_qLen(realnode->_floatInpFIFO) ;
		if (qLen > 0) {
			fTemp = RingBuffer_peekUnion(realnode->_floatInpFIFO)->f ;
			printf("%d : float length=%d , head=%f\n",__LINE__,qLen,fTemp);
		}

		qLen = RingBuffer_qLen(realnode->_int32InpFIFO) ;
		if (qLen > 0) {
			iTemp = RingBuffer_peekUnion(realnode->_int32InpFIFO)->i ;
			printf("%d : int length=%d , head=%d\n",__LINE__,qLen,iTemp);
		}

		qLen = RingBuffer_qLen(realnode->_stringInpFIFO) ;
		if (qLen > 0) {
			sTemp = (char *)RingBuffer_peekUnion(realnode->_stringInpFIFO)->p ;
			printf("%d : string length=%d , head=%s\n",__LINE__,qLen,sTemp);
		}
	}
	printf("\n");
	#endif

	return 0; /* Tell OSC we have swallowed the packet and that it should NOT try any other handlers */
}

#define OSCfuncCount  2
functions OSCcallbacks[OSCfuncCount] = {nullOSC_handler,defaultOSC_handler};
char *OSCfuncNames[OSCfuncCount] = { "", "default" };

#endif /* IPHONE */
