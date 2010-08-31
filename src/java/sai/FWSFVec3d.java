package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWSFVec3d extends FreeWRLField implements SFVec3d {
	FreeWRLBrowser browser;
	private static final int ROWS = 3;

	public FWSFVec3d(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(double[] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String rep;
		StringTokenizer tokens;

		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFVec3d getValue passed array of insufficient size");
		}

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer(RLreturn);
		}

		for (count = 0; count < ROWS; count ++) {
			value[count] = Double.valueOf(tokens.nextToken()).doubleValue();
		}
	}

	public void setValue(double[] value) throws ArrayIndexOutOfBoundsException {
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFVec3d setValue passed degenerate value");
		}
		browser.newSendEvent(this, "" + value[0] + " " + value[1] + " " + value[2]);
	}
}
