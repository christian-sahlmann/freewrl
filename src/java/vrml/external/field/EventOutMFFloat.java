// read in a multi float value from the FreeWRL browser.

package vrml.external.field;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;
import java.util.*;

public class EventOutMFFloat extends EventOutMField {
   public EventOutMFFloat() { EventType = FieldTypes.MFFLOAT; }

  public float[]  getValue() {
	float [] rval;
	int lines;
	int rows;
	int count1;
	int count2;
	StringTokenizer tokens;
	String rep;

	if (RLreturn == null) {
		rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
	} else {
		rep = RLreturn;
	}
	// get the number of lines of code to come back.

	rows = 1;

	tokens = new StringTokenizer (rep);

	//System.out.println ("DEBUG: EventOutMFFloat getValue - rep = " + rep);
	lines = Integer.valueOf(tokens.nextToken()).intValue();
	//System.out.println ("DEBUG: read in as a token " + lines);

	rval = new float [lines];

	// now, read in the lines.
	for (count1=0; count1<lines; count1++) {
		rval[count1] = Float.valueOf(tokens.nextToken()).floatValue();
	}

	// for the getSize call
	sizeof = lines;

	return rval;
  }


  public float          get1Value(int index) {
    float all[] = getValue();

    return all[index];
  }
}
