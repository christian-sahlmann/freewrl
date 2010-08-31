package vrml.node;
//JAS import java.util.Hashtable;
import vrml.*;
//
// This is the general Script class, to be subclassed by all scripts.
// Note that the provided methods allow the script author to explicitly
// throw tailored exceptions in case something goes wrong in the
// script.
//
public abstract class Script extends BaseNode
{
    public Script() {
    }

    // This method is called before any event is generated
    public void initialize() { }

    // Get a Field by name.
    //   Throws an InvalidFieldException if fieldName isn't a valid
    //   field name for a node of this type.
    protected final Field getField(String fieldName) {
	String ftype = FWJavaScript.getFieldType(this, fieldName, "field");
	if (ftype.equals("ILLEGAL"))
	   throw new InvalidFieldException(_get_nodeid()+"."+fieldName);

	/* split field type from field offset */
	String sp[] = ftype.split (" ");
	Field fval = FWCreateField.createField(sp[0]);
	fval.setOffset(ftype);


	/* read field only once, nobody except us may change it */
	FWJavaScript.readField(this, fieldName, fval);
	return fval;
   }

    // Get an EventOut by name.
    //   Throws an InvalidEventOutException if eventOutName isn't a valid
    //   eventOut name for a node of this type.
    // spec: protected
    public final Field getEventOut(String eventOutName) {
	String ftype = FWJavaScript
	    .getFieldType(this, eventOutName, "eventOut");
	if (ftype.equals("ILLEGAL"))
	    throw new InvalidEventOutException(_get_nodeid()+"."+eventOutName);

	/* split field type from field offset */
	String sp[] = ftype.split (" ");
	Field fval = FWCreateField.createField(sp[0]);
	fval.setOffset(ftype);

	fval.bind_to(new FWJavaScriptBinding(this, eventOutName, false));
	return fval;
    }

    // Get an EventIn by name.
    //   Throws an InvalidEventInException if eventInName isn't a valid
    //   eventIn name for a node of this type.
    protected final Field getEventIn(String eventInName) {
	String ftype = FWJavaScript
	    .getFieldType(this, eventInName, "eventIn");
	if (ftype.equals("ILLEGAL"))
	   throw new InvalidEventOutException(_get_nodeid()+"."+eventInName);

	/* split field type from field offset */
	String sp[] = ftype.split (" ");

	Field fval = FWCreateField.createField(sp[0]);
	fval.setOffset(ftype);

	fval.bind_to(new FWJavaScriptBinding(this, eventInName, false));
	return fval;
    }

    // processEvents() is called automatically when the script receives
    //   some set of events. It shall not be called directly except by its subclass.
    //   count indicates the number of events delivered.
    // Trevor John Thompson has submitted the following code.

    public void processEvents(final int count, final Event events[]) {
        for (int i = 0; i < count && i < events.length; ++i) {
            processEvent(events[i]);
        }
    }

    // processEvent() is called automatically when the script receives
    // an event.
    public void processEvent(Event event) { }

    // eventsProcessed() is called after every invocation of processEvents().
    public void eventsProcessed() { }

    // shutdown() is called when this Script node is deleted.
    public void shutdown() { }
}
