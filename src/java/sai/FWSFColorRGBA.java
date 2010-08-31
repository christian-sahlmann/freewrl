package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWSFColorRGBA extends FreeWRLField implements SFColorRGBA {
	FreeWRLBrowser browser;
	private static int ROWS = 4;
	
	public FWSFColorRGBA (FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(float[] value) throws ArrayIndexOutOfBoundsException {
		StringTokenizer tokens;
		String rep;
		int count;
		
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("SFColorRGBA getValue passed array of insufficient size");
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
			throw new ArrayIndexOutOfBoundsException("SFColorRGBA setValue passed degenerate rotation value");
		}
		browser.newSendEvent(this, "" + value[0] + " " + value[1] + " " + value[2] + " " + value[3]);	
	}
}
