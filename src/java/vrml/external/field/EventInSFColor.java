package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class EventInSFColor extends EventIn {

  public EventInSFColor() { EventType = FieldTypes.SFCOLOR; }

  public void          setValue(float[] value) throws
	IllegalArgumentException {

        Browser.newSendEvent (this, "" + value[0] + " " + value[1] + " " + value[2]);
  }
}
