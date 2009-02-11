#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <libFreeX3D.h>
#include <display.h>
#include <internal.h>


#include <sys/types.h>
#include "../lib/input/EAIheaders.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


#include "system.h"
#include "../lib/vrml_parser/Structs.h"
#include "../lib/main/headers.h"
#include "../lib/vrml_parser/CParseGeneral.h"
#include "../lib/scenegraph/Vector.h"
#include "../lib/vrml_parser/CFieldDecls.h"
#include "../lib/world_script/CScripts.h"
#include "../lib/vrml_parser/CParseParser.h"
#include "../lib/vrml_parser/CParseLexer.h"
#include "X3DNode.h"

