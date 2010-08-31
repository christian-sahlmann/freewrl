package sai;
import org.web3d.x3d.sai.*;

public class FWSFInt32 extends FreeWRLField implements SFInt32 {
	FreeWRLBrowser browser;
	
	public FWSFInt32(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public int getValue() {
		String rep;
		rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		return Integer.valueOf(rep).intValue();
	}
	public void setValue(int value) {
		browser.newSendEvent(this, "" + value);
	}
}
