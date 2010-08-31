package sai.eai;

import org.web3d.x3d.sai.*;
import sai.*;

import java.applet.*;
import java.net.*;
import java.io.*;


// The Thread that reads input from the FreeWRL browser...
public  class EAIinThread implements Runnable {

	// DataInputStream	EAIin;
	BufferedReader	EAIin;
	Socket		sock;
	Applet		FreeWLRSceneInterface;
	BrowserInterface 	mybrowser;

	boolean debug = false;

	// The following are used to send from the event thread to the
	// browser thread. The event thread gets stuff from the EAI port
	// from the FreeWRL Browser, and sends Replies back to the
	// browser thread.

	private PrintWriter EAItoBrowserPrintWriter = null;

	// Initialization - get the socket and the FreeWLRSceneInterfaces thread
	public EAIinThread (Socket s, Applet d, PrintWriter pwtoBrowserjava, BrowserInterface me) {

		sock = s;
		FreeWLRSceneInterface=d;
		mybrowser=me;
		EAItoBrowserPrintWriter = pwtoBrowserjava;
	}

	public void run() {
	// Open the socket, and wait for the first reply....

	String 	reply;
	String	EVentno;
	String	EVentreply;
	String	REreply;
	String	Stemp;
	String EVTime;

	try {
		EAIin = new BufferedReader( new InputStreamReader(sock.getInputStream()));
	} catch (IOException e) {
		System.out.print ("error reiniting data input stream");
	}

	// Now, this is the loop that loops to end all loops....

	try {
		// wait for FreeWRL to send us the correct number of lines...
		// rep 1, 2, 3 should be "RE" "2" "0" , with maybe another
		// parameter at the end.
		// EVs are events, and have two following lines.

		reply = new String ("");

		while (reply != null)  {
			// Loop here, processing incoming events
			reply = EAIin.readLine();

			if (reply.equals("EV")) {
				EVTime = EAIin.readLine();
				BrowserGlobals.TickTime = Double.parseDouble(EVTime);
				
				EVentno = EAIin.readLine();
				int eventno = Integer.parseInt(EVentno);
				if (debug) System.out.println ("EAIinThread 3 reply is " + EVentno);

				EVentreply = new String ("");
				reply = EAIin.readLine();
				if (debug) System.out.println ("EAIinThread 5 reply is " + reply);

				// Now, read the reply, until the string "EV_EOT is read in ???
				while (!reply.equals("EV_EOT")) {
					EVentreply = EVentreply + reply;
					reply = EAIin.readLine();
				}

				if (debug)
					System.out.println ("EAIinThread sending EVentno: " +
						EVentno + "  EventReply " + EVentreply + " reply " + reply);

				mybrowser.Browser_RL_Async_send(EVentreply,eventno);
			} else if (reply.equals("RE")) {
				EVTime = EAIin.readLine();
				BrowserGlobals.TickTime = Double.parseDouble(EVTime);

				// This is the integer reply to the command... number...
				EAItoBrowserPrintWriter.println(EAIin.readLine());

				// and the response
				EVentreply = new String ("");
				//EAItoBrowserPrintWriter.println(EAIin.readLine());
				reply = EAIin.readLine();
				if (debug) System.out.println ("EAIinThread 5xx reply is " + reply);

				// Now, read the reply, until the string "RE_EOT is read in ???
				while (!reply.equals("RE_EOT")) {
					EVentreply = EVentreply + reply;
					reply = EAIin.readLine();
				}

				EAItoBrowserPrintWriter.println(EVentreply);

				EAItoBrowserPrintWriter.flush();

			} else if (reply.equals ("QUIT")) {
				if (debug) System.out.println ("EAIinThread, got the quit signal");
				System.exit(0);
			} else {
				System.out.println ("expecting REor EV, got " + reply);
				if (debug) System.out.println ("EAIinThread 9 reply is " + reply);
			}
		}
	} catch (IOException e) {
		//System.out.print ("error reiniting data input stream\n");
	}
}
}
