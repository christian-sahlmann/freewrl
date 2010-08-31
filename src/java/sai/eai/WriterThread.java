// copyright (c) 1997,1998 stephen f. white
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

import java.io.IOException;
//JAS import java.io.EOFException;
//JAS import java.util.Vector;
//JAS import java.util.Enumeration;

class WriterThread extends Thread {
    private VFieldOutputStream		output;
    private MessageQueue		transientMessages = new MessageQueue();
    private MessageQueue		messages = new MessageQueue();
    private boolean			running;
    private boolean			timerSet;
    private long			timeout;
    private static final long		TIMEOUT = 100;
    private WriterThreadObserver	observer;

    WriterThread(VFieldOutputStream output, WriterThreadObserver observer) {
	this.output = output;
	this.observer = observer;
    }

    public void run()
    {
	running = true;
	while (running) {
            try {
		synchronized (this) {
		    if (timerSet) {
			long t = timeout - System.currentTimeMillis();
			if (t > 0) wait(t);
		    } else {
			wait();
		    }
		    if (timerSet && System.currentTimeMillis() > timeout) {
			// move all messages from transient queue to main queue
			for(;;) {
			    Message msg = transientMessages.dequeue();
			    if (msg == null) break;
			    messages.enqueue(msg);
			}
			timerSet = false;
		    }
		}
		// send all queued messages

		// this is outside the synchronized block so that new
		// messages can come in even if sendMessage() blocks
		for(;;) {
		    Message msg = messages.dequeue();
		    if (msg == null) break;
		    sendMessage(msg);
		}
            } catch (InterruptedException e) {
                running = false;
            }
	}
    }

    // this is the main access point to this object -- it enqueues
    // the given message on the appropriate queue, and wakes up the
    // sleeping thread

    public synchronized void send(Message msg)
    {
	if (msg.field < 0) {
	    messages.enqueue(msg);
	} else if (timerSet) {
	    transientMessages.enqueueUnique(msg);
	} else {
	    messages.enqueue(msg);
	    timeout = System.currentTimeMillis() + TIMEOUT;
	    timerSet = true;
	}
	notify();
    }

    // secondary access point -- stop the writer thread

    public synchronized void stopThread()
    {
	running = false;
	notify();
    }

    // sendMessage() actually sends a message (woohoo)

    private void sendMessage(Message message)
    {
	try {
	    output.writeInt(message.id);
	    output.writeShort(message.field);
	    output.writeField(message.value);
	    output.flush();
	} catch (IOException e) {
	    observer.onError(e);
	}
    }
}
