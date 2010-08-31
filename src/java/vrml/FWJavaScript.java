package vrml;

import java.net.*;
import java.lang.System;
import java.lang.reflect.*;
import java.io.*;
import java.util.Hashtable;
//JAS import java.util.Vector;
import java.util.Enumeration;
//JAS import vrml.*;
import vrml.node.*;

public final class FWJavaScript {
    static Hashtable touched = new Hashtable();
    static String reqid;
    static Browser theBrowser;

	static Socket sock;		// communication socket with FreeWRL
	static BufferedReader  EAIin;
	static PrintWriter EAIout;





    public static void add_touched(Field f) {
	touched.put(f, Boolean.TRUE);
    }

    public static void send_touched(String reqid) throws IOException {
	// System.out.println("send_touched\n");
	Enumeration e = touched.keys();
	while(e.hasMoreElements()) {
	    // System.out.println("send_touched one\n");
	    Field val = (Field) e.nextElement();
	    FWJavaScriptBinding b = val.__binding;
	    BaseNode n = b.node();
	    String f = b.field() + " " + val.getOffset();
	    //System.out.println ("java, send_touched, offset of " +
	//		    b.field() + " is " + val.getOffset());
	    String nodeid = n._get_nodeid();
	    EAIout.println("JSENDEV");
	    EAIout.println(nodeid);
	    EAIout.println(f);
	    val.__toPerl(EAIout);
	    EAIout.println();
	}
	touched.clear();
	EAIout.println("FINISHED "+reqid);
	EAIout.flush();
    }

    public static void main (String argv[])
	throws ClassNotFoundException,
	       NoSuchMethodException,
	       InstantiationException,
	       IllegalAccessException,
	       InvocationTargetException,
		Exception,
	       Throwable
    {
	int counter;
	String reply;
	String nodeid = "";
	String seqno;

  	// Create a socket here for an EAI/CLASS server on localhost
	sock = null;

	counter = 1;
	while (sock == null) {
		try {
			//System.out.println ("      ....FWJavaScript trying socket " + argv[0]);
			sock = new Socket("localhost",Integer.parseInt(argv[0]));
		} catch (IOException e) {
			// wait up to 30 seconds for FreeWRL to answer.
			counter = counter + 1;
			if (counter == 10) {
				System.out.println ("      ....FWJavaScript: Java code timed out finding FreeWRL");
				System.exit(1);
			}
			try {Thread.sleep (500);} catch (InterruptedException f) { }
		}
	}

	EAIout = new PrintWriter (sock.getOutputStream());
	EAIin = new BufferedReader( new InputStreamReader(sock.getInputStream()));

	/* Install security */
	System.out.println ("Security manager commented out");
	//System.setSecurityManager(new SecurityManager());

	/* And Go... */
	theBrowser = new Browser();

	 	Hashtable scripts = new Hashtable();
		EAIout.println("JavaClass version 1.0 - www.crc.ca");
		EAIout.flush();

		while(true) {
			String cmd = EAIin.readLine();

			// did FreeWRL leave us?
			if (cmd == null) {
				//System.out.println ("have null string  exiting...\n");
				System.exit(1);
			}

			//System.out.println("FWJ got ");
			//System.out.println("--- "+cmd);

			nodeid =EAIin.readLine();
			//System.out.println ("      ....FWJ, got nodeID " + nodeid);

			if(cmd.equals("NEWSCRIPT")) {
				String url = EAIin.readLine();
				reqid = EAIin.readLine();
				//System.out.println("NEWSCRIPT: "+url);
				FWJavaScriptClassLoader classloader =
				    new FWJavaScriptClassLoader(url);
				String classname
				    = url.substring(url.lastIndexOf('/')+1);
				if (classname.endsWith(".class"))
				    classname = classname
					.substring(0, classname.length() - 6);
				Script s;
				try {
				    s = (Script) classloader
					.loadClass(classname).newInstance();
				} catch (Exception ex) {
				    System.out.println("Can't load script: "
						       + url);
				    throw ex;
				}
				s._set_nodeid(nodeid);
				//System.out.println ("setting nodeid to " + nodeid);
				scripts.put(nodeid,s);
			} else if(cmd.equals("SETFIELD")) {
				System.out.println ("SETFIELD NOT HANDLED YET\n");

			} else if(cmd.equals("INITIALIZE")) {
				Script s = (Script)scripts.get(nodeid);
				reqid = EAIin.readLine();
				s.initialize();
				send_touched(reqid);
			} else if(cmd.equals("EVENTSPROCESSED")) {
				Script s = (Script)scripts.get(nodeid);
				reqid = EAIin.readLine();
				s.eventsProcessed();
				send_touched(reqid);
			} else if(cmd.equals("SENDEVENT")) {
				Script s = (Script)scripts.get(nodeid);
				String fname = EAIin.readLine();
				String ftype = EAIin.readLine();
				reqid = EAIin.readLine(); // note reqid position, different than
							// others, but we are using EAI functions.
							// position does not matter...
				//System.out.println ("      ....FWJ, got SENDEVENT, NodeID " + nodeid
				//		+ " field " + fname + " type " + ftype + " reqid "
				//		+ reqid);

				ConstField fval =
				    FWCreateField.createConstField(ftype);
				fval.__fromPerl(EAIin);
				double timestamp =
				    Double.parseDouble(EAIin.readLine());
				Event ev = new Event(
					fname,
					timestamp,
					fval
				);
				s.processEvent(ev);
				send_touched(reqid);
			} else {
				throw new Exception("Invalid command '"
						+ cmd + "'");
			}
		EAIout.flush();
		}
	}

    public static String getFieldType(BaseNode node, String fieldname,
				      String kind)
    {
	String str;
	try {
	    EAIout.println("GETFIELD " + node._get_nodeid() + " " + fieldname + " " + kind);
	    EAIout.flush();
	    return EAIin.readLine();
	} catch (IOException e) {
	    throw new InternalError("Communication error: "+e);
	}
    }

    public static void readField(BaseNode node, String fieldName, Field fld) {
	try {
	    FWJavaScript.EAIout.println("READFIELD " + node._get_nodeid() + " " + fieldName);
	    FWJavaScript.EAIout.flush();
	    fld.__fromPerl(EAIin);
	} catch (IOException e) {
	    throw new InternalError("Communication error: "+e);
	}
    }

    public static String getNodeType(BaseNode node)
    {
	try {
	    FWJavaScript.EAIout.println("GETTYPE "+ node._get_nodeid());
	    FWJavaScript.EAIout.flush();
	    return EAIin.readLine();
	} catch (IOException e) {
	    throw new InternalError("Communication error: "+e);
	}
    }

    public static Browser getBrowser()
    {
	return theBrowser;
    }


    public static BaseNode[] createVrmlFromString(String vrmlSyntax)
	throws InvalidVRMLSyntaxException
    {
	try {
	    FWJavaScript.EAIout.println("CREATEVRML");
	    FWJavaScript.EAIout.println(vrmlSyntax);
	    FWJavaScript.EAIout.println("EOT");
	    FWJavaScript.EAIout.flush();
	    String intstring = FWJavaScript.EAIin.readLine();
	    int number = Integer.parseInt(intstring);
	    if (number == -1)
		throw new InvalidVRMLSyntaxException(EAIin.readLine());

	    if (number == 0)
		    return null;

	    Node[] nodes = new Node[number];

	    // remember, nodes have a frontend:backend; one is known in Perl, the
	    //System.out.println ("Java: Create, reading in " + number + " nodes");
	    // other is the C pointer to memory.
	    for (int i = 0; i < number; i++)
		nodes[i] = new Node(""+EAIin.readLine()+":"+EAIin.readLine());
	    //System.out.println ("returning from Java Create");
	    return nodes;
	} catch (IOException e) {
	    throw new InternalError("Communication error: "+e);
	}
    }
}
