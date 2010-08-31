package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class EventInMFString extends EventIn {
	public EventInMFString() { EventType = FieldTypes.MFSTRING; }

	public void          setValue(String[] value) throws IllegalArgumentException {
		int count;
		String sestr;
		
		// start off the return value with the number of elements:
		sestr = "[";
		for (count = 0; count < value.length; count++) {
			if (value[count] == null) {
				throw new IllegalArgumentException();
			}
			sestr = sestr+"\"" + value[count] + "\" ";
		}
		sestr = sestr + "]";
		Browser.newSendEvent (this, sestr);
	}

	public void          set1Value(int index, String value) throws IllegalArgumentException  {
		// send index, and -1, indicating that we don't know
		// the total size of this array.
		if ((value == null) || (index < 0)) {
			throw new IllegalArgumentException();
		}

		Browser.newSendEvent(this,  " ONEVAL " + index + " \"" + value + "\"");

	}
}
