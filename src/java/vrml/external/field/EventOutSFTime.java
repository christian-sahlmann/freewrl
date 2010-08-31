package vrml.external.field;
//JAS import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;


public class EventOutSFTime extends EventOut {
	public EventOutSFTime() {EventType = FieldTypes.SFTIME;}

	public double getValue() {
                String rep;
	
		if (RLreturn == null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
		} else {
			rep = RLreturn;
		}
                return Double.valueOf(rep).doubleValue();
	}
}
