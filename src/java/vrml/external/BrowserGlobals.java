package vrml.external;
import vrml.external.field.*;
import vrml.external.FreeWRLEAI.*;
public class BrowserGlobals
{


// Events. EVno is the "highest +1" registered event number...
// EVarray corresponds to the events returned by FreeWRL  to our
// type, EVtype is the type as registered.
public static double	TickTime = 0.0;
public static int	EVno = 0;
public static int	EVarray [] = new int[256];
public static int	EVtype [] = new int[256];
public static Object EVObject[] = new Object[256];
public static EventOutObserver EVObserver[] = new EventOutObserver[256];

// The FreeWRL browser sends us changes to variables if/when they
// are updated. We tell the FreeWRL viewer what variables to look at
// by giving it a register listener command. The EAIinThread thread
// will send responses to the getVRMLReply procedure (below), or, if
// it receives an event, will send the result to the RL_Async thread...

public static EAIAsyncThread        RL_Async;

// Query Number as sent to the FreeWRL Browser.
public static int   queryno = 1;


// Sending to FreeWRL needs to synchronize on an object;
static Object FreeWRLToken = new Object();



}

