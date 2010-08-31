// read in a multi float value from the FreeWRL browser.

package vrml.external.field;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;
import java.util.*;

public class EventOutMFRotation extends EventOutMField {
   public EventOutMFRotation() { EventType = FieldTypes.MFROTATION; }

  public float[][]        getValue() {
	float [] fvals;
	float [][] rval;
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

	rows = 4;

	tokens = new StringTokenizer (rep);

	//System.out.println ("DEBUG: EventOutMFRotation getValue - rep = " + rep);
	lines = Integer.valueOf(tokens.nextToken()).intValue();
	//System.out.println ("DEBUG: read in as a token " + lines);

	rval = new float [lines][rows];

	// now, read in the lines.
	for (count1=0; count1<lines; count1++) {
		for (count2=0; count2<rows; count2++) {
			rval[count1][count2] = Float.valueOf(tokens.nextToken()).floatValue();
		}
	}

	// for getSize call
	sizeof = lines;

	return rval;
  }


  public float[]          get1Value(int index) {
    float all[][] = getValue();

    return all[index];
  }
}
