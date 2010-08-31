package vrml.external.field;
import vrml.external.field.FieldTypes;
//JAS import vrml.external.Node;
import vrml.external.Browser;

public class EventInSFInt32 extends EventIn {

  public EventInSFInt32() { EventType = FieldTypes.SFINT32; }

  public void          setValue(Integer value) {
    int count;

    Browser.newSendEvent (this, "" + value);

  return;
  }

  public void setValue(int value) {
	Integer wrapper;
	wrapper = new Integer(value);
	Browser.newSendEvent(this, "" + value);
	return;
   }
}
