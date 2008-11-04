/*******************************************************************
 *
 * FreeX3D main program
 *
 * fwopts.c
 *
 *******************************************************************/

#include "config.h"
#include "system.h"
#include "internal.h"

#include "display.h"
#include "libFreeX3D.h"

#include "options.h"

#include <getopt.h>


int parseCommandLine (int argc, char **argv)
{
  int c;
  int tmp;
  int option_index = 0;
  static const char my_optstring[] = "efghijkvlpqmnobsQWKX";
  static struct option long_options[] = {
    {"eai", 0, 0, 'e'},
    {"fast", 0, 0, 'f'},
    {"geometry", 1, 0, 'g'},
    {"help", 0, 0, 'h'},
    {"plugin", 1, 0, 'i'},
    {"fd", 1, 0, 'j'},
    {"instance", 1, 0, 'k'},
    {"version", 0, 0, 'v'},
    {"big",  0, 0, 'b'},
    {"nocollision",0, 0, 'Q'},
    {"keypress",1, 0, 'K'},
    {"eaiverbose", 0, 0, 'V'},
#ifdef DOSNAPSEQUENCE
    {"seq", 0, 0, 'l'},
    {"gif", 0, 0, 'p'},
    {"seqb",1, 0, 'm'},
    {"maximg", 1, 0, 'q'},
#endif
    {"snapb", 1, 0, 'n'},
    {"seqtmp", 1, 0, 'o'},
    {"shutter", 0, 0, 'u'},
    {"eyedist", 1, 0, 'y'},
    {"fullscreen", 0, 0, 'c'},
    {"stereoparameter", 1, 0, 't'},
    {"screendist", 1, 0, 'r'},
    {"linewidth", 1, 0, 'W'},
    {"parent", 1, 0, 'x'},
    {"server", 1, 0, 'x'},
    {"sig", 1, 0, 'x'},
    {"ps", 1, 0, 'x'},
    {0, 0, 0, 0}
  };

  c = getopt_long (argc, argv, my_optstring, long_options, &option_index);

  /**
   * TODO: options...
   */
  return TRUE;
}

