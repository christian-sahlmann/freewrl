package vrml.external.field;
import vrml.external.Node;
import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;


public class EventOutMFNode extends EventOutMField {

  // retnodes is an array of string values.
  Node[] retnodes;
	int mySize;


  public EventOutMFNode() {EventType = FieldTypes.MFNODE;}

  public Node[]      getValue() {
    String rep;
    StringTokenizer tokens;
    int counttokens;

	if (RLreturn == null) {
		rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
	} else {
		rep = RLreturn;
	}
      tokens = new StringTokenizer (rep);

    counttokens = tokens.countTokens();
    retnodes = new Node[counttokens];
    mySize = 0;

    while (mySize < counttokens) {

      retnodes[mySize] = new Node();
      rep = tokens.nextToken();
      retnodes[mySize].nodeptr = Integer.parseInt(rep);
      mySize ++;
    }

        // for the getSize call
        sizeof = mySize;

    return retnodes;
  }

  public Node        get1Value(int index) {

    // MyNode is used to ensure that the getValue call is called before this.

    Node[] MyNode = getValue();

    if ((index > sizeof) || (index < 0)) {
	System.out.println ("EventOutMFNode.get1Value - index " + index +
		" out of range");
	index = 0;
    }
    return MyNode[index];
  }
}
