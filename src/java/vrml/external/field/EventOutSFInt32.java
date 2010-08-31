package vrml.external.field;
//JAS import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;


public class EventOutSFInt32 extends EventOut {
public EventOutSFInt32() {EventType = FieldTypes.SFINT32;}

	public int           getValue() {
		String rep;
		if (RLreturn == null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
		} else {
			rep = RLreturn;
		}
		return Integer.valueOf(rep).intValue();
	}
}

