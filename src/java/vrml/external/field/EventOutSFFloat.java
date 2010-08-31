package vrml.external.field;
//JAS import java.util.*;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;


public class EventOutSFFloat extends EventOut {
	public EventOutSFFloat() {EventType = FieldTypes.SFFLOAT;}

	public float getValue() {
		String rep;

		if (RLreturn == null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
		} else {
			rep = RLreturn;
		}
		return Float.valueOf(rep).floatValue();
	}
}
