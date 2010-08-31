// Specification of the base interface for all eventOut types.
package vrml.external.field;
import vrml.external.Browser;
//JAS import vrml.external.field.FieldTypes;

public class EventOut {

 public int EventType = FieldTypes.UnknownType;
 public String inNode;	// Node to send the command to... NULL if not
 public String RLreturn;
 public String command;	// the actual command...
 public int  nodeptr =0; //pointer to start of FreeWRL structure in memory
 public int offset = 0;  //offset of actual field in memory from base.
 public int datasize = 0; // how long this data really is
 public String datatype;
 public int ScriptType = 0; // non zero indicates sending to a javascript


  //Get the type of this EventOut (specified in FieldTypes.java)
  public int           getType() {
    return EventType;
  }

   public int getIntType() {
	return EventType;
   }

  // Mechanism for setting up an observer for this field.
  // The EventOutObserver's callback gets called when the
  // EventOut's value changes.
  public void          advise(EventOutObserver f, Object userData) {

    Browser.RegisterListener (f, userData, nodeptr,offset,datatype , datasize, EventType);
  return;
  }

  // terminate notification on the passed EventOutObserver
  public void          unadvise(EventOutObserver f) {

    Browser.unRegisterListener (f, nodeptr,offset,datatype , datasize, EventType);
  return;
  }
}
