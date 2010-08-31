// Specification of the External Interface for a VRML applet browser.
// FreeWRL Viewer Interface - bypass netscape and go directly
// to the viewer.

package sai;
import org.web3d.x3d.sai.*;

import org.w3c.dom.Node;
import java.util.*;
import java.applet.*;
import java.net.*;
import java.io.*;
import vrml.external.exception.*;
import sai.eai.*;
import java.lang.System;
import javax.swing.event.*;

public class FreeWRLBrowser implements ExternalBrowser, BrowserInterface 

{
	//====================================================================
	// Threads:
		// Replies from FreeWRL.
		static Thread 		FreeWRLThread; // of type EAIinThread
		// Send commands to FreeWRL.
		static EAIoutThread 		EAIoutSender;
		// Handle Async communications from FreeWRL (eg, Regisered Listeners)
    		static EAIAsyncThread        RL_Async;

	//====================================================================
	// Communication Paths:
		// FreeWRLThread to Browser - responses from commands to FreeWRL.
    		PrintWriter EAIinThreadtoBrowser;
    		PipedWriter EAIinThreadtoBrowserPipe = null;
    		static BufferedReader BrowserfromEAI = null;
    		PipedReader BrowserfromEAIPipe = null;

	// The following are used to send to/from the FreeWRL Browser by:
	// 		- EAIinThread
	Socket	EAISocket;
	Socket		sock;
	static PrintWriter         EAIout;
	
	FreeWRLScene scene = null; // the current displayed scene

	// The following pipe listens for replies to events sent to
	// the FreeWRL VRML viewer via the EAI port.

	private String              reply = "";

	// Query Number as sent to the FreeWRL Browser.
	static int   queryno = 1;

	// Sending to FreeWRL needs to synchronize on an object;
	static Object FreeWRLToken = new Object();

	// BrowserListener list
	EventListenerList listenerList = new EventListenerList();

	//Browser description
	String description = "";

	//Disposed flag
	boolean disposed = false;

	// Interface methods.
	public int get_Browser_EVtype (int event)
	{
		return BrowserGlobals.EVtype[event];
	}

	public X3DFieldEventListener get_Browser_EVObserver (int eventno)
	{
		return BrowserGlobals.EVObserver[eventno];
	}

	public void Browser_RL_Async_send (String EVentreply, int eventno)
	{
		int EVcounter;
		for (EVcounter=0; EVcounter<BrowserGlobals.EVno; EVcounter++) {
			if (BrowserGlobals.EVarray[EVcounter] == eventno) {
				break;
			}
		}
		RL_Async.send(EVentreply, EVcounter);
	}

	// Associates this instance with the first embedded plugin in the current frame.
	public FreeWRLBrowser(Applet pApplet, int portnum) {
		int counter;
		int incrport = -1;
		EAISocket = null;
		counter = 1;

		while (EAISocket == null) {
			try {
				EAISocket = new Socket("localhost",portnum);
			} catch (IOException e) {
				// wait up to 30 seconds for FreeWRL to answer.
				counter = counter + 1;
				if (counter == 60) {
					System.out.println ("SAI: Java code timed out finding FreeWRL");
					System.exit(1);
				}
				try {
					Thread.sleep (500);
				} catch (InterruptedException f) { 
				}
			}
		}

		sock = EAISocket;       //JAS - these are the same now...

		//===================================================================
		// create the EAIinThread to Browser.
		// Open the pipe for EAI replies to be sent to us...

		try {
			EAIinThreadtoBrowserPipe = new PipedWriter();
			BrowserfromEAIPipe = new PipedReader(EAIinThreadtoBrowserPipe);
		} catch (IOException ie) {
			System.out.println ("SAI: caught error in new PipedReader: " + ie);
		}

		EAIinThreadtoBrowser = new PrintWriter(EAIinThreadtoBrowserPipe);
		BrowserfromEAI = new BufferedReader (BrowserfromEAIPipe);

		// Start the readfrom FREEWRL thread...
		//===================================================================
		// create the EAIinThread to Browser.
		// Open the pipe for EAI replies to be sent to us...

		try {
			EAIinThreadtoBrowserPipe = new PipedWriter();
			BrowserfromEAIPipe = new PipedReader(EAIinThreadtoBrowserPipe);
		} catch (IOException ie) {
			System.out.println ("SAI: caught error in new PipedReader: " + ie);
		}
	
		EAIinThreadtoBrowser = new PrintWriter(EAIinThreadtoBrowserPipe);
		BrowserfromEAI = new BufferedReader (BrowserfromEAIPipe);

		// Start the readfrom FREEWRL thread...
		FreeWRLThread = new Thread ( new EAIinThread(sock, pApplet, EAIinThreadtoBrowser, this));
		FreeWRLThread.start();

		//====================================================================
		// Start the thread that allows Registered Listenered
		// updates to come in.

		RL_Async = new EAIAsyncThread();
		RL_Async.start();

		//====================================================================
		// create the EAIoutThread - send data to FreeWRL.

		try {
			EAIout = new PrintWriter (sock.getOutputStream());
		} catch (IOException e) {
			System.out.print ("SAI: Problem in handshaking with Browser");
		}

		// Start the SendTo FREEWRL thread...
		EAIoutSender = new EAIoutThread(EAIout);
		EAIoutSender.start();

		//====================================================================
		// Browser is "gotten", and is started.
		scene = new FreeWRLScene(null, this);
		
		getRendProp();
		return;
	}

	// Associates this instance with the first embedded plugin in the current frame.
	public FreeWRLBrowser(Applet pApplet) {
		int counter;
		int incrport = -1;
		EAISocket = null;
		counter = 1;

		while (EAISocket == null) {
			try {
				EAISocket = new Socket("localhost",9877);
			} catch (IOException e) {
				// wait up to 30 seconds for FreeWRL to answer.
				counter = counter + 1;
				if (counter == 60) {
					System.out.println ("SAI: Java code timed out finding FreeWRL");
					System.exit(1);
				}
				try {
					Thread.sleep (500);
				} catch (InterruptedException f) { 
				}
			}
		}

		sock = EAISocket;	//JAS - these are the same now...

		//===================================================================
		// create the EAIinThread to Browser.
  		// Open the pipe for EAI replies to be sent to us...
        	try {
			EAIinThreadtoBrowserPipe = new PipedWriter();
			BrowserfromEAIPipe = new PipedReader(EAIinThreadtoBrowserPipe);
	        } catch (IOException ie) {
			System.out.println ("SAI: caught error in new PipedReader: " + ie);
	        }

		EAIinThreadtoBrowser = new PrintWriter(EAIinThreadtoBrowserPipe);
		BrowserfromEAI = new BufferedReader (BrowserfromEAIPipe);

  		// Start the readfrom FREEWRL thread...
	   	FreeWRLThread = new Thread ( new EAIinThread(sock, pApplet, EAIinThreadtoBrowser, this));
		FreeWRLThread.start();

		//====================================================================
		// Start the thread that allows Registered Listenered
		// updates to come in.
		RL_Async = new EAIAsyncThread();
		RL_Async.start();

		//====================================================================
		// create the EAIoutThread - send data to FreeWRL.
		try {
			EAIout = new PrintWriter (sock.getOutputStream());
		} catch (IOException e) {
			System.out.print ("SAI: Problem in handshaking with Browser");
		}

  		// Start the SendTo FREEWRL thread...
		EAIoutSender = new EAIoutThread(EAIout);
        	EAIoutSender.start();

		//====================================================================
  		// Browser is "gotten", and is started.
		scene = new FreeWRLScene(null, this);
		getRendProp();
  		return;
	}

	public void checkValid() {
		if (disposed) {
			throw new InvalidBrowserException("This browser instance is no longer valid");
		}
	}

	// Get the browser name
	public String getName() throws InvalidBrowserException, ConnectionException {
		String retval;

		checkValid();

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "K\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		return retval;
	}

	private void getRendProp() {
		String retval;
		StringTokenizer tokens;
		String key;
		String value;

		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + "X\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		tokens = new StringTokenizer (retval);
		key = "Shading";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, value);
		key = "MaxTextureSize";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, value);
		key = "TextureUnits";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, new Integer(value));
		key = "AntiAliased";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, new Boolean(value));
		key = "ColorDepth";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, new Integer(value));
		key = "TextureMemory";
		value = tokens.nextToken();
		FreeWRLRendererInfo.setRenderingProperty(key, new Float(value));
	}

	public String getVersion() throws InvalidBrowserException, ConnectionException {
		String retval;

		checkValid();

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "L\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		return retval;
	}

	// Get the current velocity of the bound viewpoint in meters/sec,
	public float getCurrentSpeed() throws InvalidBrowserException, ConnectionException {
		String retval;

		checkValid();

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "M\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		return Float.valueOf(retval).floatValue();
	}

	// Get the current frame rate of the browser, or 0.0 if not available
	public float getCurrentFrameRate() throws InvalidBrowserException, ConnectionException{
		String retval;

		checkValid();

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "N\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		return Float.valueOf(retval).floatValue();
	}

	public void replaceWorld(X3DScene passedscene) throws InvalidBrowserException, ConnectionException{
		String SysString = "";
		String retval;
		int count;
		FreeWRLNode[] nodes;

		checkValid();

		if (passedscene != null) {
			nodes = (FreeWRLNode[]) passedscene.getRootNodes();

			for (count=0; count<nodes.length; count++) {
				SysString = SysString + " " + nodes[count].getPointer();
			}
		}

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "P" + SysString);
			retval = getVRMLreply(queryno);
			queryno += 1;
		}

		((FreeWRLScene) scene).setCurrent(false);
		scene = (FreeWRLScene) passedscene;
		((FreeWRLScene) passedscene).setCurrent(true);

		browserEvent(BrowserEvent.INITIALIZED);
	}

	// Set the description of the current world in a browser-specific
	// manner. To clear the description, pass an empty string as argument
	public void setDescription(String des) throws InvalidBrowserException, ConnectionException {
		checkValid();
		description = des;
	}

	public X3DScene createX3DFromString(String str) throws InvalidBrowserException, InvalidX3DException, ConnectionException, NotSupportedException {
		FreeWRLNode[]  x = new FreeWRLNode[1];
		StringTokenizer tokens;
		String retval;
		String temp;
		int count;

		checkValid();

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" +queryno + "S "+ str +"\nEOT\n");
			retval = getVRMLreply(queryno);

			if (retval.equals("")) {
				throw new InvalidX3DException("createX3DFromString: Syntax error");
			}

			tokens = new StringTokenizer (retval);

			// We return a (node-index C-pointer) pair, so we have to divide by two        
			x = new FreeWRLNode[tokens.countTokens()/2];
			count = 0;

			// Lets go through the output, and temporarily store it        // XXX - is there a better way? ie, using a vector?
			while (tokens.hasMoreTokens()) {
				x[count] = new FreeWRLNode(this);
				x[count].setPerlPtr(tokens.nextToken());
				x[count].setPointer(tokens.nextToken());
				x[count].setType(FreeWRLFieldTypes.getIntType("h"));

				count ++;
				if (count == 100) {
					count = 99;
					System.out.println ("SAI: createVrmlFromString - warning, tied to 100 nodes");
				}
			}
			queryno += 1;
		}
		return new FreeWRLScene(x, this);
	}

	public X3DNode createNodeFromString(String str) {
		FreeWRLNode x = new FreeWRLNode(this);
		StringTokenizer tokens;
		String retval;
		String temp;
		
		synchronized(FreeWRLToken) {
			EAIoutSender.send("" + queryno + "S " + str + "\nEOT\n");
			retval = getVRMLreply(queryno);
	
			tokens = new StringTokenizer(retval);
			while(tokens.hasMoreTokens()) {
				x.setPerlPtr(tokens.nextToken());
				x.setPointer(tokens.nextToken());
				x.setType(FreeWRLFieldTypes.getIntType("h"));
			}
			queryno += 1;
		}
		return x;
	} 

	public X3DScene createX3DFromStream(InputStream is) throws InvalidBrowserException, InvalidX3DException, ConnectionException, NotSupportedException, IOException {
                FreeWRLNode[]  x = new FreeWRLNode[1];
                StringTokenizer tokens;
                String retval;
                String temp;
		String str;
                int count;

                checkValid();

		try {
			InputStreamReader isr = new InputStreamReader(is);
			str = "";
			int c;
			while ((c  = isr.read()) != -1) {
				str = str + (char)c;
			}
		} catch (Exception e) {
			throw new IOException(e + " Error reading from stream ");
		}

                synchronized (FreeWRLToken) {
                        EAIoutSender.send ("" +queryno + "S "+ str +"\nEOT\n");
                        retval = getVRMLreply(queryno);

                        if (retval.equals("")) {
                                throw new InvalidX3DException("createX3DFromStream: Syntax error");
                        }

                        tokens = new StringTokenizer (retval);

                        // We return a (node-index C-pointer) pair, so we have to divide by two
                        x = new FreeWRLNode[tokens.countTokens()/2];
                        count = 0;

                        // Lets go through the output, and temporarily store it        // XXX - is there a better way? ie, using a vector?
                        while (tokens.hasMoreTokens()) {
                                x[count] = new FreeWRLNode(this);
                                x[count].setPerlPtr(tokens.nextToken());
                                x[count].setPointer(tokens.nextToken());
                                x[count].setType(FreeWRLFieldTypes.getIntType("h"));

                                count ++;
                                if (count == 100) {
                                        count = 99;
                                        System.out.println ("SAI: createVrmlFromString - warning, tied to 100 nodes");
                                }
                        }
                        queryno += 1;
                }
                return new FreeWRLScene(x, this);
	}

	public X3DScene createX3DFromURL(String[] url) throws InvalidBrowserException, InvalidX3DException, ConnectionException, IOException {
		String retval;
		FreeWRLNode temp;
		StringTokenizer tokens;
		int count;
		ArrayList nodeList;
		int i;

		checkValid();

		nodeList = new ArrayList(1);

		if (url == null) {
			throw new InvalidX3DException ("createX3DFromURL: no URLs passed");
		}

                for (i = 0; i < url.length; i++) {
                        synchronized (FreeWRLToken) {
                                EAIoutSender.send ("" + queryno + "T " + url[i] + "\n");

                                retval = getVRMLreply(queryno);

                                tokens = new StringTokenizer (retval);
                                System.out.println("retval is: " + retval);

                                if (!retval.equals("")) {
                                        while (tokens.hasMoreTokens()) {
                                                temp = new FreeWRLNode(this);
                                                temp.setPerlPtr(tokens.nextToken());
                                                temp.setPointer(tokens.nextToken());
                                                temp.setType(FreeWRLFieldTypes.getIntType("h"));

                                                nodeList.add(temp);
                                        }
                                } else {
                                        browserEvent(BrowserEvent.URL_ERROR);
                                        throw new InvalidURLException("Browser.createX3DFromURL passed invalid URL: ");
                                }
                                queryno += 1;
                        }
                }
                FreeWRLNode[] nodes = (FreeWRLNode[]) nodeList.toArray(new FreeWRLNode[nodeList.size()]);

		FreeWRLScene scene = new FreeWRLScene(nodes, this);

		return scene;
	}

	public Map getRenderingProperties() throws InvalidBrowserException, ConnectionException {
		checkValid();
		return FreeWRLRendererInfo.getRenderingProperties();
	}
	
	public Map getBrowserProperties() throws InvalidBrowserException, ConnectionException {
		checkValid();
		return FreeWRLBrowserInfo.getBrowserProperties();
	}
	
	public void nextViewpoint() throws InvalidBrowserException, ConnectionException {
		String retval;
		checkValid();
		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + "R NEXT\n");
			retval = getVRMLreply(queryno);
			queryno++;
		}
	}
	public void previousViewpoint() throws InvalidBrowserException, ConnectionException {
		String retval;
		checkValid();
		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + "R PREV\n");
			retval = getVRMLreply(queryno);
			queryno++;
		}
	}
	public void firstViewpoint() throws InvalidBrowserException, ConnectionException  {
		String retval;
		checkValid();
		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + "R FIRST\n");
			retval = getVRMLreply(queryno);
			queryno++;
		}
	}
	public void lastViewpoint() throws InvalidBrowserException, ConnectionException  {
		String retval;
		checkValid();
		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + "R LAST\n");
			retval = getVRMLreply(queryno);
			queryno++;
		}
	}
	public void print(Object obj) throws InvalidBrowserException, ConnectionException  {
		checkValid();
		System.out.print(obj);
	}
	public void println(Object obj) throws InvalidBrowserException, ConnectionException  {
		checkValid();
		System.out.println(obj);
	}

    // Add and delete, respectively, a route between the specified eventOut
    // and eventIn of the given nodes
    public String addRoute(FreeWRLNode fromNode, String fromEventOut,
                                  FreeWRLNode toNode, String toEventIn) throws
				  IllegalArgumentException {
      String retval;

      synchronized (FreeWRLToken) {
         EAIoutSender.send ("" + queryno + "H " + fromNode.getPointer() + " " + fromEventOut +
		" " + toNode.getPointer() + " " + toEventIn +"\n");
         retval = getVRMLreply(queryno);
         queryno += 1;
       }
      return retval;
    }


    public String deleteRoute(FreeWRLNode fromNode, String fromEventOut,
                                     FreeWRLNode toNode, String toEventIn)
			 throws IllegalArgumentException {
      String retval;

      synchronized (FreeWRLToken) {
         EAIoutSender.send ("" + queryno + "J " + fromNode.getPointer() + " " + fromEventOut +
		" " + toNode.getPointer() + " " + toEventIn +"\n");
         retval = getVRMLreply(queryno);
         queryno += 1;
       }
      return retval;
    }

    // begin and end an update cycle
    public void          beginUpdate() {}
    public void          endUpdate() {}

    // called after the scene is loaded, before the first event is processed
    public void initialize() {
      System.out.println ("SAI: Initialize Not Implemented");

    }


    // called just before the scene is unloaded
    public void shutdown() {
      System.out.println ("SAI: Shutdown Not Implemented");

    }

	// Get a DEFed node by name. Nodes given names in the root scene
	// graph must be made available to this method. DEFed nodes in inlines,
	// as well as DEFed nodes returned from createVrmlFromString/URL, may
	// or may not be made available to this method, depending on the
	// browser's implementation

	public X3DNode getNode (String NodeName) throws NodeUnavailableException 
	{
		FreeWRLNode temp;
		temp = new FreeWRLNode(this);
		String retval;
		StringTokenizer tokens;

                if (NodeName == null) {
                        throw new NodeUnavailableException(NodeName + " undefined");
                } 

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "A " + NodeName + "\n");
			System.out.println("SENT: " + NodeName + " " + queryno);
			retval = getVRMLreply(queryno);
			System.out.println("GOT RETVAL: " + retval);
			tokens = new StringTokenizer(retval);
			temp.setPerlPtr(tokens.nextToken());
			temp.setPointer(tokens.nextToken());
			queryno += 1;
		}
		if (((temp.getPerlPtr()).equals(NodeName)) || ((temp.getPerlPtr()).equals("-1"))) {
			throw new NodeUnavailableException(NodeName + " undefined");
		}
		return temp;
	}


	// Send Event to the VRML Browser. Note the different methods, depending
	// on the parameters.
	public static void SendChildEvent (String parent, String offset, String FieldName, String Child) {
		String retval;

		// System.out.println ("SendChildEvent: sending to " + parent + " ofs:" +
		//	offset + " field " + FieldName + " child: " + Child);

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "C " + parent + " " + offset + " " +
				FieldName + " "+ Child + "\n");
			retval = getVRMLreply(queryno);
			queryno += 1;
		}
		return;
	}

	// Most events don't need us to wait around for it.
	public static void newSendEvent (FreeWRLField field, String Value) {

		synchronized (FreeWRLToken) {
			EAIoutSender.send ("" + queryno + "D" + field.getDataType()+ " " +
				field.getNodePtr()+ " " + field.getOffset() + " " +field.getScriptType() + " " + Value + "\n");
			queryno += 1;
		}
		return;
	}

	public static String sendGlobalCommand (String command) {
		String retval;
		
		synchronized (FreeWRLToken) {
			EAIoutSender.send("" + queryno + command);
			retval = getVRMLreply(queryno);
			queryno++;
		}
		
		return retval;
	}


  protected static String SendEventType (String NodeName, String ptr, String FieldName, String direction) {

      // get a type from a particular node.

      String retval;

      synchronized (FreeWRLToken) {
        EAIoutSender.send ("" + queryno + "F " + NodeName + " " + ptr + " " + FieldName + " " + direction + "\n");
        retval = getVRMLreply(queryno);
        queryno += 1;
      }
      return retval;
}

  public static String SendEventOut (String nodeptr, String offset, String datasize, String datatype,
	String command) {

      String retval;
       synchronized (FreeWRLToken) {
         EAIoutSender.send ("" + queryno + "E " + nodeptr + " " + offset + " " + datatype +
 		" " + datasize + "\n");

         retval = getVRMLreply(queryno);
         queryno += 1;
      }
     return retval;
}

  public static void RegisterListener (X3DFieldEventListener f, Object userData,
			String nodeptr, String offset, String datatype,  String datasize, int EventType)
    {
       synchronized (FreeWRLToken) {
         EAIoutSender.send ("" + queryno + "G " + nodeptr + " " + offset + " " + datatype + " " + datasize + "\n");

         BrowserGlobals.EVarray [BrowserGlobals.EVno] =  queryno;
         BrowserGlobals.EVtype [BrowserGlobals.EVno] = EventType;
         BrowserGlobals.EVObject[BrowserGlobals.EVno] = userData;
         BrowserGlobals.EVObserver[BrowserGlobals.EVno] = f;

         BrowserGlobals.EVno += 1;

         getVRMLreply(queryno);
         queryno += 1;
       }
    }

    public static void unRegisterListener (X3DFieldEventListener f, String nodeptr, String offset, String datatype,  String datasize, int EventType)
    {
       synchronized (FreeWRLToken) {
         EAIoutSender.send ("" + queryno + "W " + nodeptr + " " + offset + " " + datatype + " " + datasize + "\n");

         getVRMLreply(queryno);
         queryno += 1;
       }
    }

	protected synchronized static String getVRMLreply (int queryno) {
		String req = "0";
	        String rep = "";

 		// This waits for the correct event toe be returned. Note that
		// sendevents dont wait, so there is the possibility that
		// we will have to while away a bit of time waiting...

		while (queryno != Integer.parseInt(req)) {
			try {
				req = BrowserfromEAI.readLine();
				System.out.println("req " + req);
			} catch (IOException ie) {
				System.out.println ("SAI: caught " + ie);
				return rep;
			}
	
			if (queryno != Integer.parseInt(req)) {
				System.out.println ("SAI: getVRMLreply - REPLIES MIXED UP!!! Expecting " + queryno + " got " + req);
			}

			try {
				rep = BrowserfromEAI.readLine();
				System.out.println("rep is " + rep);
			} catch (IOException ie) { System.out.println ("SAI: getVRMLreply failed"); return null; }
		}
		return rep;
	}

	public void close() {
		System.out.println("SAI: closing socket");
		//JAS try {
			System.exit(1);
			//JAS EAIoutSender.stopThread();
			//JAS EAISocket.close();
			//JAS EAIfromFreeWRLStream.close();

		//JAS } catch (IOException e) {
		//JAS }
	}

	public void dispose() {
		disposed = true;
		browserEvent(BrowserEvent.SHUTDOWN);
	}

	public void addBrowserListener(BrowserListener listener) throws InvalidBrowserException, ConnectionException  {
		listenerList.add(BrowserListener.class, listener);
	}

	public void removeBrowserListener(BrowserListener listener) throws InvalidBrowserException, ConnectionException  {
		listenerList.remove(BrowserListener.class, listener);
	}

	public void browserEvent(int type) {
		BrowserEvent theEvent;

		theEvent = new BrowserEvent(this, type);

		Object[] listeners = listenerList.getListenerList();

		for (int i = listeners.length-2; i >=0; i-=2) {
			if (listeners[i] == BrowserListener.class) {
				((BrowserListener)listeners[i+1]).browserChanged(theEvent);
			}
		}
	}

	public X3DScene currentScene() {
		return scene;
	}
        public ProfileInfo getProfile(String name) throws ConnectionException, InvalidBrowserException, NotSupportedException {
		checkValid();
		return (ProfileInfo) FWProfInfo.getProfile(name);
	}
	public ProfileInfo[] getSupportedProfiles() throws InvalidBrowserException, ConnectionException {
		checkValid();
		return (ProfileInfo[]) FWProfInfo.getProfiles();
	}
        public ComponentInfo[] getSupportedComponents() throws InvalidBrowserException, ConnectionException  {
		checkValid();
		return (ComponentInfo[]) FWProfInfo.getComponents();
	}
        public ComponentInfo getComponent(String name, int level) throws InvalidBrowserException, NotSupportedException, ConnectionException  {
		checkValid();
		return (ComponentInfo) FWProfInfo.getComponent(name, level);
	}
        public X3DExecutionContext getExecutionContext() throws InvalidBrowserException, ConnectionException {
		checkValid();
		return (X3DExecutionContext) scene;
	}
        public X3DScene createScene(ProfileInfo profile, ComponentInfo[] components) throws InvalidBrowserException, ConnectionException  {
		checkValid();
		FWComponentInfo comp;

		if ((profile == null) && (components == null)) {	
			return null;
		}

		if (profile != null) {
			ProfileInfo prof = FWProfInfo.getProfile(profile.getName());
		}
		
		if (components != null) {
			for (int i = 0; i < components.length; i++) {
				comp = FWProfInfo.getComponent(components[i].getName(), components[i].getLevel());
			}
		}

		return new FreeWRLScene((FWComponentInfo[]) components, (FWProfileInfo) profile, this);
	}
        public void loadURL(String[] url, Map parameters) throws InvalidBrowserException, InvalidURLException, ConnectionException {
		// Figure out if we are going to replace the scene or add to it.
		boolean replace;
		String retval;
		FreeWRLNode temp;
		StringTokenizer tokens;
		int count;
		int globalcount;
		ArrayList nodeList;
		int i;

		checkValid();

		count = 0;
		if (parameters == null) {
			replace = true;
		} else {
			Boolean rtemp;
			rtemp = (Boolean) parameters.get("replace");
			if (rtemp == null) {
				replace = true;
			} else {
				replace = rtemp.booleanValue();
			}
		}

		nodeList = new ArrayList(1);

		for (i = 0; i < url.length; i++) {
			synchronized (FreeWRLToken) {
				EAIoutSender.send ("" + queryno + "T " + url[i] + "\n");

				retval = getVRMLreply(queryno);

				tokens = new StringTokenizer (retval);
				System.out.println("retval is: *" + retval + "*");
			
				if (!retval.equals("")) {
					while (tokens.hasMoreTokens()) {
						temp = new FreeWRLNode(this);
						temp.setPerlPtr(tokens.nextToken());
						temp.setPointer(tokens.nextToken());
						temp.setType(FreeWRLFieldTypes.getIntType("h"));

						nodeList.add(temp);
					}
				} else {
					browserEvent(BrowserEvent.URL_ERROR);
					throw new InvalidURLException("Browser.loadURL passed invalid URL: ");
				}	
				queryno += 1;
			}
		}
		FreeWRLNode[] nodes = (FreeWRLNode[]) nodeList.toArray(new FreeWRLNode[nodeList.size()]);

		if (!(nodeList.size() == 0)) {
			if (replace) {
				FreeWRLScene newscene = new FreeWRLScene(nodes, this);
				replaceWorld(newscene);
			} else {
				if (scene == null) {
					scene = new FreeWRLScene(this);
				}
				for (i = 0; i < nodes.length; i++) {
					scene.addRootNode(nodes[i]);
				} 
			}
			browserEvent(BrowserEvent.INITIALIZED);
		}
	}
        public String getDescription() throws InvalidBrowserException, ConnectionException  {
		checkValid();
		return description;
	}
	public void stopRender() {
	}
	public void pauseRender() {
	}
	public X3DScene importDocument(Node element) throws InvalidBrowserException, InvalidDocumentException, NotSupportedException, ConnectionException {
		throw new NotSupportedException("FreeWRL does not yet support this function");
	}

}

