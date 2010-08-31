package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class EventInSFVec2f extends EventIn {
  public EventInSFVec2f() { EventType = FieldTypes.SFVEC2F; };

  public void          setValue(float[] value) throws IllegalArgumentException {
    if (value.length < 2) {
	throw new IllegalArgumentException();
   }
    Browser.newSendEvent (this, "" + value[0] + " " + value[1]);

  return;
  }

}
