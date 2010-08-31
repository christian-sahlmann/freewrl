package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Node;
import vrml.external.Browser;

public class EventInSFNode extends EventIn {

  public EventInSFNode() { EventType = FieldTypes.SFNODE; }

  public void          setValue(Node node) {
    int count;

    Browser.newSendEvent (this, "" +node.nodeptr);

  return;
  }
}
