/*
  =INSERT_TEMPLATE_HERE=

  $Id: options.c,v 1.7 2009/02/05 10:33:04 couannette Exp $

  FreeX3D command line arguments.

*/

#include <config.h>
#include <system.h>
#include <internal.h>

#include <libFreeX3D.h>

#include "main.h"
#include "options.h"

#if HAVE_GETOPT_H
# include <getopt.h>
#endif

extern int wantEAI;

void print_version()
{
    const char *libver, *progver;

    libver = libFreeX3D_get_version();
    progver = freex3d_get_version();
    
    printf("Program version: %s\nLibrary version: %s\n", progver, libver);
    printf("\nFreeWRL VRML/X3D browser from CRC Canada (http://www.crc.ca)\n");
    printf("   type \"man freewrl\" to view man pages\n\n");
}

void usage()
{
    printf( "usage: freex3d [options] <VRML or X3D file|URL>\n\n"
	    "\t-h|--help        This help.\n"
	    "\t-v|--version     Print version.\n"
	    "\nWindow options:\n"
	    "\t-c|--fullscreen  Set window fullscreen\n"
	    "\t-g|--geometry    Set window geometry.\n"
	    "\t-b|--big         Set window size to 800x600.\n"
	    "\nGeneral options:\n"
	    "\t-e|--eai         Enable EAI.\n"
	    "\t-f|--fast        Set global texture size to -256 (fast).\n"
	    "\t-W|--linewidth   Set line width.\n"
	    "\t-Q|--nocollision Disable collision management.\n"
	    "\nSnapshot options:\n"
	    "\t-p|--gif         Set file format to GIF (default is PNG).\n"
	    "\t-n|--snapfile    Set file name pattern.\n"
	    "\t-o|--snaptmp     Set directory for snap files.\n"
#if defined(DOSNAPSEQUENCE)
	    "\nSnapshot sequence options:\n"
	    "\t-l|--seq         Set snapshot sequence mode.\n"
	    "\t-m|--seqfile     Set sequence file name pattern.\n"
	    "\t-q|--maximg      Set maximum number of files in sequence.\n"
#endif
	    "\nMisc options:\n"
	    "\t-V|--eaiverbose  Set EAI subsystem messages.\n"
	    "\t-r|--screendist  Set screen distance.\n"
	    "\t-y|--eyedist     Set eye distance.\n"
	    "\t-u|--shutter     Set shutter glasses.\n"
	    "\t-t|--stereo      Set stereo parameter.\n"
	    "\t-K|--keypress    Set immediate key pressed when ready.\n"
	    "\nInternal options:\n"
	    "\t-i|--plugin      Called from plugin.\n"
	    "\t-j|--fd          Pipe to command the program.\n"
	    "\t-k|--instance    Instance of plugin.\n"
	    ""
	);
}

int parseCommandLine (int argc, char **argv)
{
    int c;
    float ftmp;
    int option_index = 0;

    static const char optstring[] = "efg:hijkvlpqmnobsQWKX";

    static struct option long_options[] = {
	{"eai", 0, 0, 'e'},
	{"fast", 0, 0, 'f'},
	{"geometry", required_argument, 0, 'g'},
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
	{"seqfile",1, 0, 'm'},
	{"maximg", 1, 0, 'q'},
#endif
	{"snapfile", 1, 0, 'n'},
	{"snaptmp", 1, 0, 'o'},
	{"shutter", 0, 0, 'u'},
	{"eyedist", 1, 0, 'y'},
	{"fullscreen", 0, 0, 'c'},
	{"stereo", 1, 0, 't'},
	{"screendist", 1, 0, 'r'},
	{"linewidth", 1, 0, 'W'},
	
	{"parent", 1, 0, 'x'},
	{"server", 1, 0, 'x'},
	{"sig", 1, 0, 'x'},
	{"ps", 1, 0, 'x'},
	{0, 0, 0, 0}
    };

#if HAVE_GETOPT_LONG_ONLY
    FW_DEBUG("Using getopt_long_only\n");
#else
# if HAVE_GETOPT_LONG
    FW_DEBUG("Using getopt_long\n");
# else
    FW_DEBUG("Using getopt\n");
# endif
#endif

    while (1) {

	/* Do we want getopt to print errors by itself ? */
	opterr = 0;

#if HAVE_GETOPT_LONG_ONLY
	c = getopt_long_only(argc, argv, optstring, long_options, &option_index);
#else
# if HAVE_GETOPT_LONG
	c = getopt_long(argc, argv, optstring, long_options, &option_index);
# else
	c = getopt(argc, argv, optstring);
# endif
#endif

	if (c == -1)
	    break;

	switch (c) {
	case '?': /* getopt error: unknown option or missing argument */
	    FW_ERROR("ERROR: unknown option or missing argument to option: %c\n", optopt);
	    exit(1);
	    break;
#if 0
/* Glibc bug currently prevents us from using this code to better trap errors.
   ASA the bug is fixed we can uncomment our code. */
	case ':': /* getopt error: missing argument */
	    FW_ERROR("ERROR: missing arguement for option: %c\n", optopt);
	    exit(1);
	    break;
#endif
	case 0:
	    printf ("FreeWRL option --%s", long_options[option_index].name);
	    if (optarg)
		printf (" with arg %s", optarg);
	    printf ("\n");
	    break;

	case 'h': /* --help */
	    usage();
	    exit(0);
	    break;

	case 'v': /* --version */
	    print_version();
	    exit(0);
	    break;

	case 'x': /* non-implemented option */
	    printf("option --%s not implemented yet, complain bitterly\n",
		   long_options[option_index].name);
	    break;

	case 'e': /* --eai */
	    wantEAI = TRUE;
	    break;

	case 'f': /* --fast */
	    /* set negative so that the texture thread will pick this up */
	    setTexSize(-256);
	    break;

	case 'g': /* --geometry */
	    if (!optarg) {
		FW_ERROR("Argument missing for option -g/--geometry\n");
		exit(1);
	    } else {
		setGeometry_from_cmdline(optarg);
	    }
	    break;

	case 'c': /* --fullscreen */
#if defined(HAVE_XF86_VMODE)
	    fullscreen = 1;
#else
	    printf("\nFullscreen mode is only available when xf86vmode extension is\n"
		  "supported by your X11 server: i.e. XFree86 version 4 or later,\n"
		   "Xorg version 1.0 or later.\n"
		   "Configure should autodetect it for you. If not please report"
		   "this problem to\n\t " PACKAGE_BUGREPORT "\n");
	    fullscreen = 0;
#endif
	    break;

	case 'i': /* --plugin */
	    sscanf(optarg,"pipe:%d",&_fw_pipe);
	    isBrowserPlugin = TRUE;
	    break;

	case 'j': /* --fd */
	    sscanf(optarg,"%d",&_fw_browser_plugin);
	    break;

	case 'k': /* --instance */
	    sscanf(optarg,"%u",&_fw_instance);
	    break;

	case 'W': /* --linewidth */
	    /* Petr Mikiluk - ILS line width */
	    sscanf(optarg,"%g", &ftmp);
	    setLineWidth(ftmp);
	    break;

	case 'Q': /* --nocollision */
	    be_collision = FALSE;
	    break;

	    /* Snapshot stuff */
#ifdef DOSNAPSEQUENCE
	case 'l': /* --seq */
	    setSnapSeq();
	    break;

	case 'm': /* --seqfile */
	    setSeqFile(optarg);
	    break;

	case 'q': /* --maximg */
	    sscanf(optarg,"%d",&maxSnapImages);
	    setMaxImages(maxSnapImages);
	    break;
#endif
	case 'p': /* --gif */
	    setSnapGif();
	    break;

	case 'n': /* --snapfile*/
	    setSnapFile(optarg);
	    break;

	case 'o': /* --snaptmp */
	    setSeqTemp(optarg);
	    break;

	case 'V': /* --eaiverbose */
	    setEaiVerbose();
	    break;

	case 'b': /* --big */
	    /* Alberto Dubuc - bigger window */
	    setGeometry_from_cmdline("800x600");
	    break;

	case 'r': /* --screendist */
	    /* Shutter patches from Mufti @rus */
	    setScreenDist(optarg);
	    break;

	case 't': /* --stereo */
	    setStereoParameter(optarg);
	    break;

	case 'u': /* --shutter */
	    setShutter();
	    setXEventStereo();
	    break;

	case 'y': /* --eyedist */
	    setEyeDist(optarg);
	    break;

	case 'K': /* --keypress */
	    /* initial string of keypresses once main url is loaded */
	    keypress_string = optarg;
	    break;

	default:
	    FW_ERROR("ERROR: getopt returned character code 0%o, unknown error.\n", c);
	    exit(1);
	    break;
	}
    }

    if (optind < argc) {
	if (optind != (argc-1)) {
	    FW_WARN("freewrl:warning, expect only 1 file on command line; running file: %s\n",
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

