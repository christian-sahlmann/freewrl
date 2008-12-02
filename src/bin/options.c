/*
  =INSERT_TEMPLATE_HERE=

  $Id: options.c,v 1.4 2008/12/02 17:41:38 couannette Exp $

  FreeX3D command line arguments.

*/

#include <config.h>
#include <system.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "options.h"

#include <getopt.h>

extern int wantEAI;


int parseCommandLine (int argc, char **argv)
{
    int c;
    int tmp;
    int option_index = 0;
    static const char my_optstring[] = "efghijkvlpqmnobsQWKX";

    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"eai", 0, 0, 'e'},
	    {"fast", 0, 0, 'f'},
	    {"geometry", 1, 0, 'g'},
	    {"help", 0, 0, 'h'},
	    {"plugin", 1, 0, 'i'},
	    {"fd", 1, 0, 'j'},
	    {"instance", 1, 0, 'k'},
	    {"version", 0, 0, 'v'},
	    {"big",  0, 0, 'b'},		/* Alberto Dubuc */
	    {"nocollision",0, 0, 'Q'},		/* Alberto Dubuc */
	    {"keypress",1, 0, 'K'},		/* Robert Sim */
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
	    {"linewidth", 1, 0, 'W'},  /* Petr Mikulik */

	    {"parent", 1, 0, 'x'},
	    {"server", 1, 0, 'x'},
	    {"sig", 1, 0, 'x'},
	    {"ps", 1, 0, 'x'},
	    {0, 0, 0, 0}
	};

	c = getopt_long (argc, argv, my_optstring, long_options, &option_index);

	if (c == -1)
	    break;

	switch (c) {
	case 0:
	    printf ("FreeWRL option --%s", long_options[option_index].name);
	    if (optarg)
		printf (" with arg %s", optarg);
	    printf ("\n");
	    break;

	case 'x':
	    printf ("option --%s not implemented yet, complain bitterly\n",
		    long_options[option_index].name);
	    break;

	case 'e':
	    wantEAI=TRUE;
	    break;

	case 'f':
	    /* set negative so that the texture thread will pick this up */
	    setTexSize(-256);
	    break;

	case 'g':
	    setGeometry(optarg);
	    break;

	case 'c':
	    fullscreen = 1;
#if HAVE_XF86_VMODE
	    printf("\nFullscreen mode is only available for XFree86 version 4.\n");
	    printf("If you are running version 4, please add -DXF86V4 to your vrml.conf file\n");
	    printf("in the FREEWRL_DEFINES section, and add -lXxf86vm to the FREEWRL_LIBS section.\n");
	    fullscreen = 0;
#endif
	    break;

	case 'h':
	    printf ("\nFreeWRL VRML/X3D browser from CRC Canada (http://www.crc.ca)\n");
	    printf ("   type \"man freewrl\" to view man pages\n\n");
	    break;

	case 'i': sscanf (optarg,"pipe:%d",&_fw_pipe); isBrowserPlugin = TRUE; break;
	case 'j': sscanf (optarg,"%d",&_fw_browser_plugin);  break;
	case 'k': sscanf (optarg,"%u",&_fw_instance); break;
	case 'v': printf ("FreeWRL version: %s\n", freex3d_get_version()); 
	    exit(0);break;
	    /* Petr Mikiluk - ILS line width */
	case 'W': sscanf (optarg,"%g",&tmp); setLineWidth(tmp); break;

	case 'Q': be_collision = FALSE; break;


	    /* Snapshot stuff */
#ifdef DOSNAPSEQUENCE
	case 'l': setSnapSeq(); break;
	case 'p': snapGif = TRUE; break;
	case 'm':
	    setSeqFile(optarg);
	    break;
	case 'q': sscanf (optarg,"%d",&maxSnapImages);
	    setMaxImages(maxSnapImages);
	    break;

#endif

	case 'n':
	    setSnapFile(optarg);
	    break;
	case 'o':
	    setSeqTemp(optarg);
	    break;

	case 'V':
	    setEaiVerbose();
	    break;

	case 'b': /* Alberto Dubuc - bigger window */
	    setGeometry ("800x600");
	    break;

	    /* Shutter patches from Mufti @rus */
	case 'r':
	    setScreenDist(optarg);
	    break;
	case 't':
	    setStereoParameter(optarg);
	    break;
	case 'u':
	    setShutter();
	    XEventStereo();
	    break;
	case 'y':
	    setEyeDist(optarg);
	    break;

	    /* initial string of keypresses once main url is loaded */
	case 'K':
	    keypress_string = optarg;
	    break;
	default:
	    /* printf ("?? getopt returned character code 0%o ??\n", c); */
	    break;
	}
    }

    if (optind < argc) {
	if (optind != (argc-1)) {
	    printf ("freewrl:warning, expect only 1 file on command line; running file: %s\n",
		    argv[optind]);
	}

	/* save the url for later use, if required */
	setFullPath(argv[optind]);
    } else {
	/* printf ("no options  - just make BrowserFullPath point to nothing\n"); */
	setFullPath("");
	return FALSE;
    }
    return TRUE;
}

