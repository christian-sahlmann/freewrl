// Interface for Browser.java
//

package vrml.external;

import vrml.external.field.EventOutObserver;

public interface BrowserInterface {

public int get_Browser_EVtype (int event);
public EventOutObserver get_Browser_EVObserver (int eventno);
public void Browser_RL_Async_send(String EVentreply, int eventno);


}

