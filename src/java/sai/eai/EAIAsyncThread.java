// copyright (c) 1997,1998 stephen f. white
// Modified for use with EAI and FreeWRL. John Stewart CRC Canada 1999
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
package sai.eai;


//JAS import java.util.*;
//JAS import java.awt.*;
//JAS import java.net.*;
//JAS import java.io.*;
import org.web3d.x3d.sai.*;
import vrml.external.field.*;
import sai.*;
import java.util.*;

// John A. Stewart - john.stewart@crc.ca
//
// This sends "Registered Listeners" replies back to the EAI code. It
// is called by the EAIinThread; and it queues these messages, just in case
// some of the global tables are blocked by another process.


public class EAIAsyncThread extends Thread {
    private EAIAsyncQueue			EAIMessages = new EAIAsyncQueue();
    private boolean			running;
    private boolean			timerSet;
    private long			timeout;
    private static final long		TIMEOUT = 100;
    private WriterThreadObserver	observer;

    public void run()
    {
	running = true;
	while (running) {
            try {
		synchronized (this) {

			wait ((long) 50);
		}
		// send all queued EAIMessages

		// this is outside the synchronized block so that new
		// EAIMessages can come in even if sendEAIAsyncMessage() blocks
		for(;;) {
		    EAIAsyncMessage msg = EAIMessages.dequeue();
		    if (msg == null) break;
		    sendEAIAsyncMessage(msg);
		}
            } catch (InterruptedException e) {
                running = false;
            }
	}
	//System.out.println("EAIAsyncThread exiting");
    }

    // this is the main access point to this object -- it enqueues
    // the given EAIMessage on the appropriate queue, and wakes up the
    // sleeping thread

    public synchronized void send(String  eaistring, int indx)
    {
	EAIAsyncMessage msg;
        msg = new EAIAsyncMessage(eaistring,indx);

	EAIMessages.enqueue(msg);
	notify();
    }

    // secondary access point -- stop the writer thread

    public synchronized void stopThread()
    {
		System.out.println("stopping EAIAsyncThread");
	running = false;
	notify();
    }

    // sendEAIAsyncMessage() actually sends a EAIAsyncMessage (woohoo)

    private void sendEAIAsyncMessage(EAIAsyncMessage msg)
    {
      float[] fvals = new float[4];
      int count = 0;
      EventOut me;

      //System.out.println ("EAIAsyncThread.callback - value " + msg.value +
	//		" EventType " + msg.EventNumber );

      if (BrowserGlobals.EVtype[msg.EventNumber]==18) {
	me = new EventOutSFVec3f();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==12) {
        me = new EventOutSFRotation();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==11) {
        me = new EventOutMFNode();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==3) {
        me = new EventOutSFTime();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==1) {
        me = new EventOutSFBool();

      // These are not yet properly handled...
      //  public final static int SFIMAGE      = 2;
      //  public final static int SFTIME       = 3;
      //  public final static int SFCOLOR      = 4;
      //  public final static int MFCOLOR      = 5;
      //  public final static int SFFLOAT      = 6;
      //  public final static int MFFLOAT      = 7;
      //  public final static int SFINT32      = 8;
      //  public final static int MFINT32      = 9;
      //  public final static int SFNODE       = 10;
      //  public final static int MFROTATION   = 13;
      //  public final static int SFSTRING     = 14;
      //  public final static int MFSTRING     = 15;
      //  public final static int SFVEC2F      = 16;
      //  public final static int MFVEC2F      = 17;
      //  public final static int MFVEC3F      = 19;


      } else {
	System.out.println (" EAIASyncThread: handling something funny here, " +
		BrowserGlobals.EVtype[msg.EventNumber]);
        me = new EventOut();
      }
      me.RLreturn = msg.value;

      if (BrowserGlobals.EVObserver[msg.EventNumber] != null) {
	X3DFieldEvent event = new X3DFieldEvent(me, BrowserGlobals.TickTime, BrowserGlobals.EVObject[msg.EventNumber]);
      	BrowserGlobals.EVObserver[msg.EventNumber].readableFieldChanged (event);
      } else {
		System.out.println ("WARNING - EAIAsyncThread.callback - thread callback null, discarding");
      }
    }
}
