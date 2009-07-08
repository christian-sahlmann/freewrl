/*
  =INSERT_TEMPLATE_HERE=

  $Id: options.c,v 1.11.2.1 2009/07/08 21:55:04 couannette Exp $

  FreeWRL command line arguments.

*/

#include <config.h>
#include <system.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "main.h"
#include "options.h"

#if HAVE_GETOPT_H
# include <getopt.h>
#endif

extern int wantEAI;

void print_version()
{
    const char *libver, *progver;

    libver = libFreeWRL_get_version();
    progver = freewrl_get_version();
    
    printf("Program version: %s\nLibrary version: %s\n", progver, libver);
    printf("\nFreeWRL VRML/X3D browser from CRC Canada (http://www.crc.ca)\n");
    printf("   type \"man freewrl\" to view man pages\n\n");
}

void usage()
{
    printf( "usage: freewrl [options] <VRML or X3D file|URL>\n\n"
	    "  -h|--help               This help.\n"
	    "  -v|--version            Print version.\n"
	    "\nWindow options:\n"
	    "  -c|--fullscreen         Set window fullscreen\n"
	    "  -g|--geometry <WxH>     Set window geometry (W width, H height).\n"
	    "  -b|--big                Set window size to 800x600.\n"
	    "\nGeneral options:\n"
	    "  -e|--eai                Enable EAI.\n"
	    "  -f|--fast               Set global texture size to -256 (fast).\n"
	    "  -W|--linewidth <float>  Set line width.\n"
	    "  -Q|--nocollision        Disable collision management.\n"
	    "\nSnapshot options:\n"
	    "  -p|--gif                Set file format to GIF (default is PNG).\n"
	    "  -n|--snapfile <string>  Set output file name pattern with <string>,\n"
	    "                          (use %%n for iteration number).\n"
	    "  -o|--snaptmp <string>   Set output directory for snap files.\n"
#if defined(DOSNAPSEQUENCE)
	    "\nSnapshot sequence options:\n"
	    "  -l|--seq                Set snapshot sequence mode.\n"
	    "  -m|--seqfile <string>   Set sequence file name pattern.\n"
	    "  -q|--maximg <number>    Set maximum number of files in sequence.\n"
#endif
	    "\nMisc options:\n"
	    "  -V|--eaiverbose         Set EAI subsystem messages.\n"
	    "  -r|--screendist <float> Set screen distance.\n"
	    "  -y|--eyedist <float>    Set eye distance.\n"
	    "  -u|--shutter            Set shutter glasses.\n"
	    "  -t|--stereo <float>     Set stereo parameter (angle factor).\n"
	    "  -K|--keypress <string>  Set immediate key pressed when ready.\n"
	    "\nInternal options:\n"
	    "  -i|--plugin <string>    Called from plugin.\n"
	    "  -j|--fd <number>        Pipe to command the program.\n"
	    "  -k|--instance <number>  Instance of plugin.\n"
	    ""
	);
}

const char * validate_string_arg(const char *optarg)
{
    return NULL; /* TODO: implement validate_* functions */
}

int parseCommandLine (int argc, char **argv)
{
    int c;
    float ftmp;
    int option_index = 0;
    int real_option_index;
    const char *real_option_name;

#if defined(DOSNAPSEQUENCE)
    static const char optstring[] = "efg:hi:j:k:vVlpq:m:n:o:bsQW:K:Xcr:y:ut";
#else
    static const char optstring[] = "efg:hi:j:k:vVpn:o:bsQW:K:Xcr:y:ut";
#endif

    static struct option long_options[] = {

/* { const char *name, int has_arg, int *flag, int val }, */

	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'v'},

	{"fullscreen", no_argument, 0, 'c'},
	{"geometry", required_argument, 0, 'g'},
	{"big", no_argument, 0, 'b'},

	{"eai", no_argument, 0, 'e'},
	{"fast", no_argument, 0, 'f'},
	{"linewidth", required_argument, 0, 'W'},
	{"nocollision", no_argument, 0, 'Q'},

	{"gif", no_argument, 0, 'p'},
	{"snapfile", required_argument, 0, 'n'},
	{"snaptmp", required_argument, 0, 'o'},

#if defined(DOSNAPSEQUENCE)
	{"seq", no_argument, 0, 'l'},
	{"seqfile", required_argument, 0, 'm'},
	{"maximg", required_argument, 0, 'q'},
#endif

	{"eaiverbose", no_argument, 0, 'V'},
	{"screendist", required_argument, 0, 'r'},
	{"eyedist", required_argument, 0, 'y'},
	{"shutter", no_argument, 0, 'u'},
	{"stereo", no_argument, 0, 't'},
	{"keypress", required_argument, 0, 'K'},

	{"plugin", required_argument, 0, 'i'},
	{"fd", required_argument, 0, 'j'},
	{"instance", required_argument, 0, 'k'},

	{0, 0, 0, 0}
    };

    int find_opt_for_optopt(char c) {
	int i = 0;
	struct option *p;
	p = &(long_options[i]);
	while (p->name) {
	    if (!p->flag) {
		if (p->val == c) {
		    return i;
		}
	    }
	    p = &(long_options[++i]);
	}
	return -1;
    }

    while (1) {

	/* Do we want getopt to print errors by itself ? */
	opterr = 0;

# if HAVE_GETOPT_LONG
	c = getopt_long(argc, argv, optstring, long_options, &option_index);
# else
	c = getopt(argc, argv, optstring);
# endif

	if (c == -1)
	    break;

	if ((c == '?')) {
	    real_option_index = find_opt_for_optopt(optopt);
	} else {
	    real_option_index = find_opt_for_optopt(c);
	}
	if (real_option_index < 0) {
	    real_option_name = argv[optind-1];
	} else {
	    real_option_name = long_options[real_option_index].name;
	}
/* 	FW_DEBUG("option_index=%d optopt=%c option=%s\n", real_option_index, c, */
/* 		 real_option_name); */

	switch (c) {

	    /* Error handling */

	case '?': /* getopt error: unknown option or missing argument */
	    ERROR_MSG("ERROR: unknown option or missing argument to option: %c (%s)\n", 
		     c, real_option_name);
	    exit(1);
	    break;

	    /* Options handling */

	case 'h': /* --help, no argument */
	    usage();
	    exit(0);
	    break;

	case 'v': /* --version, no argument */
	    print_version();
	    exit(0);
	    break;

/* Window options */

	case 'c': /* --fullscreen, no argument */
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

	case 'g': /* --geometry, required argument: string "WxH" */
	    if (!optarg) {
		ERROR_MSG("Argument missing for option -g/--geometry\n");
		exit(1);
	    } else {
		setGeometry_from_cmdline(optarg);
	    }
	    break;

	case 'b': /* --big, no argument */
	    setGeometry_from_cmdline("800x600");
	    break;

/* General options */

	case 'e': /* --eai, no argument */
	    wantEAI = TRUE;
	    break;

	case 'f': /* --fast, no argument */
	    /* set negative so that the texture thread will pick this up */
	    setTexSize(-256);
	    break;

	case 'W': /* --linewidth, required argument: float */
	    sscanf(optarg,"%g", &ftmp);
	    setLineWidth(ftmp);
	    break;

	case 'Q': /* --nocollision, no argument */
	    be_collision = FALSE;
	    break;

/* Snapshot options */

	case 'p': /* --gif, no argument */
	    setSnapGif();
	    break;

	case 'n': /* --snapfile, required argument: string */
	    setSnapFile(optarg);
	    break;

	case 'o': /* --snaptmp, required argument: string */
	    setSnapTmp(optarg);
	    break;

/* Snapshot sequence options */

#if defined(DOSNAPSEQUENCE)
	case 'l': /* --seq, no argument */
	    setSnapSeq();
	    break;

	case 'm': /* --seqfile, required argument: string */
	    setSeqFile(optarg);
	    break;

	case 'q': /* --maximg, required argument: number */
	    sscanf(optarg,"%d",&maxSnapImages);
	    setMaxImages(maxSnapImages);
	    break;
#endif

/* Misc options */

	case 'V': /* --eaiverbose, no argument */
	    setEaiVerbose();
	    break;

	case 'r': /* --screendist, required argument: float */
	    setScreenDist(optarg);
	    break;

	case 'y': /* --eyedist, required argument: float */
	    setEyeDist(optarg);
	    break;

	case 'u': /* --shutter, no argument */
	    setShutter();
	    setXEventStereo();
	    break;

	case 't': /* --stereo, required argument: float */
	    setStereoParameter(optarg);
	    break;

	case 'K': /* --keypress, required argument: string */
	    /* initial string of keypresses once main url is loaded */
	    keypress_string = optarg; /* ! strdup ! */
	    break;

/* Internal options */

	case 'i': /* --plugin, required argument: number */
	    sscanf(optarg,"pipe:%d",&_fw_pipe);
	    isBrowserPlugin = TRUE;
	    break;

	case 'j': /* --fd, required argument: number */
	    sscanf(optarg,"%d",&_fw_browser_plugin);
	    break;

	case 'k': /* --instance, required argument: number */
	    sscanf(optarg,"%u",(unsigned int *)&_fw_instance);
	    break;


	default:
	    ERROR_MSG("ERROR: getopt returned character code 0%o, unknown error.\n", c);
	    exit(1);
	    break;
	}
    }

    if (optind < argc) {
	if (optind != (argc-1)) {
	    WARN_MSG("WARNING: expect only 1 file on command line; running file: %s\n",
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

