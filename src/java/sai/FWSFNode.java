package sai;
import org.web3d.x3d.sai.*;

public class FWSFNode extends FreeWRLField implements SFNode {
	FreeWRLBrowser browser;
	
	public FWSFNode(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public X3DNode getValue() {
		return null;
	}
	public void setValue(X3DNode value) throws InvalidNodeException {
		browser.newSendEvent(this, ((FreeWRLNode)value).getPointer());
	}
}
