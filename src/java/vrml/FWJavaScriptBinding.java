package vrml;
import vrml.FWJavaScript;
//JAS import java.io.IOException;

public class FWJavaScriptBinding {
    BaseNode node;
    String fieldName;
    String lastUpdate;
    boolean doUpdateRead;

    public FWJavaScriptBinding(BaseNode n, String f) {
	this(n,f,true);
    }

    public FWJavaScriptBinding(BaseNode n, String f, boolean u) {
	node = n; fieldName = f;
	doUpdateRead = u;
    }
    public BaseNode node() {return node;}
    public String field() {return fieldName;}

    public void updateRead(Field field) {
	if (!doUpdateRead || lastUpdate == FWJavaScript.reqid)
	    return;
	FWJavaScript.readField(node, fieldName, field);
	lastUpdate = FWJavaScript.reqid;
    }

    public void updateWrite(Field field) {
	FWJavaScript.add_touched(field);
	lastUpdate = FWJavaScript.reqid;
    }

    public String toString() {
	return node._get_nodeid()+"."+fieldName;
    }
}
