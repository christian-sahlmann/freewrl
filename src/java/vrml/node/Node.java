package vrml.node;
//JAS import java.util.Hashtable;
import vrml.Field;
import vrml.ConstField;
import vrml.BaseNode;
import vrml.FWJavaScript;
import vrml.*;

//
// This is the general Node class
//
public class Node extends BaseNode
{
    public Node(String id) {
	super(id);
    }


    // Get an EventIn by name. Return value is write-only.
    //   Throws an InvalidEventInException if eventInName isn't a valid
    //   eventIn name for a node of this type.
    public final Field getEventIn(String eventInName) {

	String ftype = FWJavaScript
	    .getFieldType(this, eventInName, "eventIn");
	if (ftype.equals("ILLEGAL"))
	    throw new InvalidEventInException(_get_nodeid()+"."+eventInName);


	/* split field type from field offset */
	System.out.println ("eventIn: " + ftype);
	String sp[] = ftype.split (" ");
	// type is in sp[0]
	Field fval = FWCreateField.createField(sp[0]);
	fval.setOffset(ftype);
	fval.bind_to(new FWJavaScriptBinding(this, eventInName));
	return fval;
    }

    // Get an EventOut by name. Return value is read-only.
    //   Throws an InvalidEventOutException if eventOutName isn't a valid
    //   eventOut name for a node of this type.
    public final ConstField getEventOut(String eventOutName) {
	String ftype = FWJavaScript
	    .getFieldType(this, eventOutName, "eventOut");

	if (ftype.equals("ILLEGAL"))
	    throw new InvalidEventOutException(_get_nodeid()+"."+eventOutName);

	/* split field type from field offset */
	String sp[] = ftype.split (" ");

	ConstField fval = FWCreateField.createConstField(sp[0]);
	fval.setOffset(ftype);
	fval.bind_to(new FWJavaScriptBinding(this, eventOutName));
	return fval;
    }

    // Get an exposed field by name.
    //   Throws an InvalidExposedFieldException if exposedFieldName isn't a valid
    //   exposedField name for a node of this type.
    public final Field getExposedField(String exposedFieldName) {
	String ftype = FWJavaScript
	    .getFieldType(this, exposedFieldName, "exposedField");
	if (ftype.equals("ILLEGAL"))
	    throw new InvalidExposedFieldException(_get_nodeid()+"."
						   +exposedFieldName);

	/* split field type from field offset */
	String sp[] = ftype.split (" ");

	Field fval = FWCreateField.createField(sp[0]);
	fval.setOffset(ftype);

	fval.bind_to(new FWJavaScriptBinding(this, exposedFieldName));
	return fval;
    }
}


