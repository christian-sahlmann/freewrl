package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWSFColor extends FreeWRLField implements SFColor {
	FreeWRLBrowser browser;
	private static int ROWS = 3;

	public FWSFColor(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(float[] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String rep;
		StringTokenizer tokens;

		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFColor getValue passed array of insufficient length");
		}

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer(RLreturn);
		}

		for (count = 0; count < ROWS; count++) {
			value[count] = Float.valueOf(tokens.nextToken()).floatValue();
		}
	}

	public void setValue(float[] value) throws IllegalArgumentException, ArrayIndexOutOfBoundsException {
		int count;
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFColor setValue passed degenerate colour value");
		}
		for (count = 0; count < ROWS; count++) {
			if ((value[count] < 0) || (value[count] > 1)) {
				throw new IllegalArgumentException("SFColor setValue passed invalid colour value");
			}
		}	
		browser.newSendEvent(this, "" + value[0] + " " + value[1] + " " + value[2]);
	}
}
