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
//JAS import java.applet.*;
//JAS import java.awt.*;
//JAS import java.net.*;
import java.io.*;



//import java.io.IOException;
//import java.io.EOFException;
//import java.util.Vector;
//import java.util.Enumeration;

public class EAIoutThread extends Thread {
    private PrintWriter			output;
    private EAIoutQueue			transientEAIMessages = new EAIoutQueue();
    private EAIoutQueue			EAIMessages = new EAIoutQueue();
    private boolean			running;
    private boolean			timerSet;
    private long			timeout;
    private static final long		TIMEOUT = 100;
    private WriterThreadObserver	observer;

    public EAIoutThread(PrintWriter output) {
	this.output = output;
    }

    public void run()
    {
	running = true;
	while (running) {
            try {
		synchronized (this) {
			wait((long) 50);
		}
		// send all queued EAIMessages

		for(;;) {
		    EAIMessage msg = EAIMessages.dequeue();
		    if (msg == null) break;
		    sendEAIMessage(msg);
		}
            } catch (InterruptedException e) {
                running = false;
            }
	}
    }

    // this is the main access point to this object -- it enqueues
    // the given EAIMessage on the appropriate queue, and wakes up the
    // sleeping thread

    public synchronized void send(String  eaistring)
    {
	EAIMessage msg;

        msg = new EAIMessage(eaistring);

	EAIMessages.enqueue(msg);
	notify();
    }

    // secondary access point -- stop the writer thread

    public synchronized void stopThread()
    {
	running = false;
	notify();
    }

    // sendEAIMessage() actually sends a EAIMessage (woohoo)

    private void sendEAIMessage(EAIMessage msg)
    {
		if (msg == null) { stopThread(); return; }
	    output.println(msg.mmm);
	    output.flush();
    }
}
