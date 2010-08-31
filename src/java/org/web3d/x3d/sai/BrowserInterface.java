// Interface for Browser.java
//

package org.web3d.x3d.sai;

public interface BrowserInterface {

public int get_Browser_EVtype (int event);
public X3DFieldEventListener get_Browser_EVObserver (int eventno);
public void Browser_RL_Async_send(String EVentreply, int eventno);
}

