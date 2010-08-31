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
package vrml.external.FreeWRLEAI;


//JAS import java.util.*;
//JAS import java.awt.*;
//JAS import java.net.*;
//JAS import java.io.*;
import vrml.external.field.*;
import vrml.external.*;

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

      if (BrowserGlobals.EVtype[msg.EventNumber]==19) {
        me = new EventOutMFVec3f();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==18) {
	me = new EventOutSFVec3f();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==17) {
        me = new EventOutMFVec2f();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==16) {
        me = new EventOutSFVec2f();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==15) {
        me = new EventOutMFString();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==14) {
        me = new EventOutSFString();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==13) {
        me = new EventOutMFRotation();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==12) {
        me = new EventOutSFRotation();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==11) {
        me = new EventOutMFNode();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==10) {
        me = new EventOutSFNode ();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==9) {
        me = new EventOutMFInt32();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==8) {
        me = new EventOutSFInt32();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==7) {
        me = new EventOutMFFloat();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==6) {
        me = new EventOutSFFloat();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==5) {
        me = new EventOutMFColor();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==4) {
        me = new EventOutSFColor();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==3) {
        me = new EventOutSFTime();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==2) {
        me = new EventOutSFImage();
      } else if (BrowserGlobals.EVtype[msg.EventNumber]==1) {
        me = new EventOutSFBool();


      } else {
	System.out.println (" EAIASyncThread: handling something funny here, " +
		BrowserGlobals.EVtype[msg.EventNumber]);
        me = new EventOut();
      }
      me.RLreturn = msg.value;

      if (BrowserGlobals.EVObserver[msg.EventNumber] != null) {
      	BrowserGlobals.EVObserver[msg.EventNumber].callback (me,
		BrowserGlobals.TickTime, BrowserGlobals.EVObject[msg.EventNumber]);
      } else {
		System.out.println ("WARNING - EAIAsyncThread.callback - thread callback null, discarding");
      }
    }
}
