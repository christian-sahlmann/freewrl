package vrml.external.field;
import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class EventOutSFRotation extends EventOut {
	public EventOutSFRotation() {EventType = FieldTypes.SFROTATION;}

	public float[]       getValue() {


		float[] fvals = new float[4];
		StringTokenizer tokens;
		String rep;

		if (RLreturn == null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
		} else {
			rep = RLreturn;
		}
		tokens = new StringTokenizer (rep);

		fvals[0]=Float.valueOf(tokens.nextToken()).floatValue();
		fvals[1]=Float.valueOf(tokens.nextToken()).floatValue();
		fvals[2]=Float.valueOf(tokens.nextToken()).floatValue();
		fvals[3]=Float.valueOf(tokens.nextToken()).floatValue();

		return fvals;
	}
}
