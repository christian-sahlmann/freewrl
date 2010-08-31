package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class  EventInSFFloat extends EventIn {
  public EventInSFFloat() { EventType = FieldTypes.SFFLOAT; }

  public void          setValue(float value) {
    Browser.newSendEvent (this, "" + value);
    return;
  }
}
