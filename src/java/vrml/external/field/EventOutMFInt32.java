// read in a multi Int32 value from the FreeWRL browser.

package vrml.external.field;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;
import java.util.*;

public class EventOutMFInt32 extends EventOutMField {
   public EventOutMFInt32() { EventType = FieldTypes.MFINT32; }

  public int[]  getValue() {
	int [] rval;
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

	//System.out.println ("DEBUG: EventOutMFInt32 getValue - rep = " + rep);
	lines = Integer.valueOf(tokens.nextToken()).intValue();
	//System.out.println ("DEBUG: read in as a token " + lines);

	rval = new int [lines];

	// now, read in the lines.
	for (count1=0; count1<lines; count1++) {
		rval[count1] = Integer.valueOf(tokens.nextToken()).intValue();

	}

	// for the getSize call
	sizeof = lines;
	return rval;
  }


  public int          get1Value(int index) {
    int all[] = getValue();

    return all[index];
  }
}
