package sai;
import org.web3d.x3d.sai.*;

public class FWSFDouble extends FreeWRLField implements SFDouble {
	FreeWRLBrowser browser;
	
	public FWSFDouble (FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public double getValue() {
		String rep;
		rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		return Double.valueOf(rep).doubleValue();
	}

	public void setValue(double value) {
		browser.newSendEvent(this, "" + value);
	}
}
