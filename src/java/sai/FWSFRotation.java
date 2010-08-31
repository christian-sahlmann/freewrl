package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWSFRotation extends FreeWRLField implements SFRotation {
	FreeWRLBrowser browser;
	private static int ROWS = 4;
	
	public FWSFRotation(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(float[] value) throws ArrayIndexOutOfBoundsException {
		StringTokenizer tokens;
		String rep;
		int count;
		
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFRotation getValue passed array of insufficient size");
		}

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer (RLreturn);
		}

		for (count = 0; count < ROWS; count++) {
			value[count] = Float.valueOf(tokens.nextToken()).floatValue();
		}		
	}
	public void setValue(float[] value) throws ArrayIndexOutOfBoundsException {
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFRotation setValue passed degenerate rotation value");
		}
		browser.newSendEvent(this, "" + value[0] + " " + value[1] + " " + value[2] + " " + value[3]);	
	}
}
