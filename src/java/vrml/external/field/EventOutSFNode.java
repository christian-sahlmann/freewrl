package vrml.external.field;
import java.util.*;
import vrml.external.Browser;
import vrml.external.field.FieldTypes;
import vrml.external.Node;


public class EventOutSFNode extends EventOut {
  public EventOutSFNode() {EventType = FieldTypes.SFNODE;}

  public Node  getValue() {
    String rep;
    StringTokenizer tokens;
    int counttokens;

  Node retnode;

	if (RLreturn == null) {
		rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
	} else {
		rep = RLreturn;
	}
      tokens = new StringTokenizer (rep);
    counttokens = tokens.countTokens();


      retnode = new Node();
      rep = tokens.nextToken();
      retnode.nodeptr = Integer.parseInt(rep);
    return retnode;

  }
}

