%module vrmlExternal
 %{
 /* Includes the header in the wrapper code */
#define SWIG_FILE_WITH_INIT 
 #include "EAI_swigMe.h"
 %}
 

 /* Parse the header file to generate wrappers */
 %include "EAI_swigMe.h"
