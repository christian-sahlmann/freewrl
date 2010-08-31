package sai;
import org.web3d.x3d.sai.*;

public class FWSFTime extends FreeWRLField implements SFTime {
	FreeWRLBrowser browser;

	public FWSFTime(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public double getValue() {
		return  -1.0;
	}

	public long getJavaValue() {
		return  -1;
	}

	public void setValue(double value) {
		browser.newSendEvent(this, "" + value);
	}

	public void setValue(long value) {
	}
}
