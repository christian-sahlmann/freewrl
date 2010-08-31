package org.web3d.x3d.sai;

import java.util.*;

public class BrowserEvent extends EventObject {
	public static final int INITIALIZED = 0;
	public static final int SHUTDOWN = 1;
	public static final int URL_ERROR = 2;
	public static final int CONNECTION_ERROR = 10;
	public static final int LAST_IDENTIFIER = 100;

        int action;
	Object browser;

	public BrowserEvent(Object b, int a) {
		super(b);
		browser = b;
		action = a;
	}
	
	public int getID() {
		return action;
	}
}
