package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWSFVec3f extends FreeWRLField implements SFVec3f {
	FreeWRLBrowser browser;
	private static final int ROWS = 3;

	public FWSFVec3f(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(float[] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String rep;
		StringTokenizer tokens;

		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFVec3f getValue passed array of insufficient size");
		}

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer(RLreturn);
		}

		for (count = 0; count < ROWS; count ++) {
			value[count] = Float.valueOf(tokens.nextToken()).floatValue();
		}
	}

	public void setValue(float[] value) throws ArrayIndexOutOfBoundsException {
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFVec3f setValue passed degenerate value");
		}
		browser.newSendEvent(this, "" + value[0] + " " + value[1] + " " + value[2]);
	}
}
