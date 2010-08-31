// Interface which all observer classes must implement.
package vrml.external.field;

import vrml.external.field.EventOut;
//JAS import vrml.external.Browser;


public interface EventOutObserver {
  void callback(EventOut value, double timeStamp, Object userData) ;

}
