package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Node;
import vrml.external.Browser;

public class EventInMFNode extends EventIn {

  public EventInMFNode() { EventType = FieldTypes.MFNODE; }

  public void          setValue(Node[] node) throws IllegalArgumentException {
    int count;

    for (count = 0; count < node.length; count++) {
	if (node[count].nodeptr == 0) {
		throw new IllegalArgumentException();
	}
      Browser.SendChildEvent (nodeptr,offset, command, node[count].nodeptr);
    }
  return;
  }

  public void          set1Value(int index, Node node) throws IllegalArgumentException {
	if (node.nodeptr == 0) {
		throw new IllegalArgumentException();
	}

	Browser.newSendEvent(this, " ONEVAL " + index + " " + node.nodeptr);

  return;
  }
}
