package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;


public class EventInSFString extends EventIn {
  public EventInSFString() { EventType = FieldTypes.SFSTRING;}

  public void          setValue(String value) {
    Browser.newSendEvent (this, value.length() + ":" + value + " ");
    return;
  }
}
