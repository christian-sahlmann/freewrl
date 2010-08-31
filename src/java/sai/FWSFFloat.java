package sai;
import org.web3d.x3d.sai.*;

public class FWSFFloat extends FreeWRLField implements SFFloat {
	FreeWRLBrowser browser;
	
	public FWSFFloat(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public float getValue() {
		String rep;
		rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		return Float.valueOf(rep).floatValue();
	}

	public void setValue(float value) {
		browser.newSendEvent(this, "" + value);
	}
}
