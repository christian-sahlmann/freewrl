package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Node;
import vrml.external.Browser;

public class EventInMFVec2f extends EventIn {

  public EventInMFVec2f() { EventType = FieldTypes.MFVEC2F; }

  public void          setValue(float[][] value) throws IllegalArgumentException {
	int count;
	String val;

	if (value == null) {
		throw new IllegalArgumentException();
	}
	val = "[";

	for (count = 0; count < value.length; count++) {
		if (value[count].length < 2) {
			throw new IllegalArgumentException();
		}
		val = val + " " + value[count][0] + " " + value[count][1] + ",";
	}
	val = val + "]";
	Browser.newSendEvent(this,val);
	
    return;
  }

  public void          set1Value(int index, float value[]) throws IllegalArgumentException {
	if ((value == null) || (index < 0) || (value.length < 2)) {
		throw new IllegalArgumentException();
	}
	Browser.newSendEvent(this,  " ONEVAL " + index + " " + value[0] + " " + value[1]);

  return;
  }
}
