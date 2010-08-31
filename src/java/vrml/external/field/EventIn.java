// Specification of the base interface for all eventIn types.
package vrml.external.field;
import vrml.external.field.*;

public class EventIn {

  int EventType = FieldTypes.UnknownType;
  public String command;
  public String inNode;
  public int datasize = 0;
  public int nodeptr = 0;
  public int offset = 0;
  public int ScriptType = 0;
  public String datatype;

  // Get the type of this EventIn (specified in FieldTypes.java)
  //public int           getType() {
  //	return EventType;
  //}
	public int getIntType() {
		return EventType;
	}

	public int getType() {
		return EventType;
	}
}
