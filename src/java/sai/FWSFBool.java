package sai;
import org.web3d.x3d.sai.*;

public class FWSFBool extends FreeWRLField implements SFBool {
	FreeWRLBrowser browser;
	
	public FWSFBool(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public boolean getValue() throws InvalidFieldException {
		checkValid();
		return RLreturn.equals("TRUE");
	}

	public void setValue(boolean value) throws InvalidFieldException {
		checkValid();
		if (value) {
			browser.newSendEvent(this, "TRUE");
		} else {
			browser.newSendEvent(this, "FALSE");
		}
	}
}
