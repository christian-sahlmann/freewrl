package vrml.external.field;
//JAS import java.util.*;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;


public class EventOutSFBool extends EventOut {
	public EventOutSFBool() {EventType = FieldTypes.SFBOOL;}

	public boolean       getValue() {
        String rep;

	if (RLreturn == null) {
		rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
	} else {
		rep = RLreturn;
	}
	return rep.equals("TRUE");
	}
}
