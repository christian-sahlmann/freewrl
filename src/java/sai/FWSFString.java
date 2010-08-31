package sai;
import org.web3d.x3d.sai.*;

public class FWSFString extends FreeWRLField implements SFString {
	FreeWRLBrowser browser;

	public FWSFString(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public String getValue() {
		return null;
	}
	public void setValue(String value) {
		browser.newSendEvent(this, "\"" + value + "\"");
	}
}
