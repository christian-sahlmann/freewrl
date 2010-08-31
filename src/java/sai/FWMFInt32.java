package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWMFInt32 extends FreeWRLMField implements MFInt32 {
	FreeWRLBrowser browser;
	private static final int ROWS = 1;
	
	public FWMFInt32(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(int[] values) throws ArrayIndexOutOfBoundsException {
		int lines;
		int count1; 
		StringTokenizer tokens;
		String rep;
	
		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		tokens = new StringTokenizer(rep);
		lines = Integer.valueOf(tokens.nextToken()).intValue();
		
		if (values.length < lines) {
			throw new ArrayIndexOutOfBoundsException("MFInt32 getValue passed array of insufficient size");
		}

		for (count1=0; count1 < lines; count1++) {
			values[count1] = Integer.valueOf(tokens.nextToken()).intValue();
		}	
	}

	public int get1Value(int index) throws ArrayIndexOutOfBoundsException {
		int lines;
		int count1; 
		StringTokenizer tokens;
		String rep;
		int[] rval;
	
		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		tokens = new StringTokenizer(rep);
		lines = Integer.valueOf(tokens.nextToken()).intValue();

		rval = new int[lines];
		
		if (index > lines) {
			throw new ArrayIndexOutOfBoundsException("MFInt32 get1Value passed index out of bounds");
		}

		for (count1=0; count1 < lines; count1++) {
			rval[count1] = Integer.valueOf(tokens.nextToken()).intValue();
		}	
		return rval[index];
	}

	public void setValue(int size, int[] value) {
		int count;
		String val;

		if (size > value.length) {
			size = value.length;
		}

		val = " " + size;
	
		for (count = 0; count < size; count++) {
			val = val + " " + value[count];
		}
		browser.newSendEvent(this, val);
	}

	public void set1Value(int index, int value) throws ArrayIndexOutOfBoundsException {
		browser.newSendEvent(this, " ONEVAL " + index + " " + value);
	}

	public void append(int[] value) {
                int lines;
                int append_size;
                int count1, count2;
                StringTokenizer tokens;
                String rep;
                String val;

                if (RLreturn == null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                } else {
                        rep = RLreturn;
                }

                tokens = new StringTokenizer(rep);
                lines = Integer.valueOf(tokens.nextToken()).intValue();

                append_size = value.length;

                val = " " + (lines + append_size);

                for (count1 = 0; count1 < lines; count1++) {
                        val = val + tokens.nextToken();
                }

                for (count1 = 0; count1 < append_size; count1++) {
                        val = val + " " + value[count1];
                }

                browser.newSendEvent(this, val);
	}
	public void insertValue(int index, int[] value) {
                int lines;
                int insert_size;
                int count1, count2;
                StringTokenizer tokens;
                String rep;
                String val;

                if (RLreturn == null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                } else {
                        rep = RLreturn;
                }

                tokens = new StringTokenizer(rep);
                lines = Integer.valueOf(tokens.nextToken()).intValue();

                insert_size = value.length;

                if ((index > lines) || (index < 0)) {
                        throw new ArrayIndexOutOfBoundsException("MFInt32 insertValue passed index out of bounds");
                }

                val = " " + (lines + insert_size);

                for (count1 = 0; count1 < index; count1++) {
                        val = val + tokens.nextToken();
                }

                for (count1 = 0; count1 < insert_size; count1++) {
                        val = val + " " + value[count1];
                }

                for (count1 = index; count1 < lines; count1++) {
                        val = val + tokens.nextToken();
                }

                browser.newSendEvent(this, val);
	}
}
