package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWMFRotation extends FreeWRLMField implements MFRotation {
	FreeWRLBrowser browser;
	static final int ROWS = 4;
	
	public FWMFRotation(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(float[][] value) throws ArrayIndexOutOfBoundsException {
		int lines;
		int count1;
		int count2;
		StringTokenizer tokens;
		String rep;

		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		tokens = new StringTokenizer(rep);

		lines = Integer.valueOf(tokens.nextToken()).intValue();
		
		if (value.length < lines) {
			throw new ArrayIndexOutOfBoundsException("MFRotation getValue passed array of insufficient size");
		}
		
		for (count1 = 0; count1 < lines; count1++) {
			for (count2=0; count2<ROWS; count2++) {
				value[count1][count2] = Float.valueOf(tokens.nextToken()).floatValue();
			}
		}
	}

	public void getValue(float[] value) throws ArrayIndexOutOfBoundsException {
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
		
		if (value.length < (lines*ROWS)) {
			throw new ArrayIndexOutOfBoundsException("MFRotation getValue passed array of insufficient size");
		}
		
		for (count1 = 0; count1 < (lines*ROWS); count1++) {
			value[count1] = Float.valueOf(tokens.nextToken()).floatValue();
		}
	}

	public void get1Value(int index, float[] value) {
		int lines;
		float [][] tval;
		int count1;
		int count2;
		StringTokenizer tokens;
		String rep;

		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		tokens = new StringTokenizer(rep);

		lines = Integer.valueOf(tokens.nextToken()).intValue();
		
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("MFRotation get1Value passed array of insufficient size");
		}
		if (index > lines) {
			throw new ArrayIndexOutOfBoundsException("MFRotation get1Value passed index out of bounds");
		}
	
		tval = new float[lines][ROWS];
		
		for (count1 = 0; count1 < lines; count1++) {
			for (count2=0; count2<ROWS; count2++) {
				tval[count1][count2] = Float.valueOf(tokens.nextToken()).floatValue();
			}
		}

		for (count1 = 0; count1 < ROWS; count1++) {
			value[count1] = tval[index][count1];
		}
	}

	public void setValue(int numRotations, float[] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String val;

		if (numRotations > (value.length % ROWS)) {
			numRotations = (value.length % ROWS);
		}

		if ((numRotations*ROWS) != (value.length)) {
			throw new ArrayIndexOutOfBoundsException("MFRotation setValue passed degenrate rotation value");
		}

		val = " " + numRotations;

		for (count = 0; count < (numRotations * ROWS); count++) {
			val = val + " " + value[count];
		}
		
		browser.newSendEvent(this, val);
	}

	public void setValue(int numRotations, float[][] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String val;
		
		if (numRotations > value.length) {
			numRotations = value.length;
		}

		val = " " + numRotations;
		for (count = 0; count < numRotations; count++) {
			if (value[count].length < ROWS) {
				throw new ArrayIndexOutOfBoundsException("MFRotation setValue passed degenerate rotation value");
			}	
			val = val + " " + value[count][0] + " " + value[count][1] + " " + value[count][2] + " " + value[count][3];
		}
		browser.newSendEvent(this, val);
	}
	public void set1Value(int index, float[] value) {
		browser.newSendEvent(this, " ONEVAL " + index + " " + value[0] + " " + value[1] + " " + value[2] + " " + value[3]);
	}

	public void append(float[] value) {
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
                                
                if ((append_size % ROWS)!= 0) {
                        throw new ArrayIndexOutOfBoundsException("Rotation value array contains insufficient number of colors");
                }
                
                val = " " + (lines + (append_size/ROWS));

                for (count1 = 0; count1 < (lines*ROWS); count1++) {
                        val = val + tokens.nextToken();
                }

                for (count1 = 0; count1 < append_size; count1++) {
                        val = val + " " + value[count1];
                }

                browser.newSendEvent(this, val);
	}
	public void insertValue(int index, float[] value) {
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

                if ((insert_size % ROWS) != 0) {
                        throw new ArrayIndexOutOfBoundsException("Rotation value array contains insufficient number of colors");
                }

                if ((index > (lines/ROWS)) || (index < 0)) {
                        throw new ArrayIndexOutOfBoundsException("MFRotation insertValue passed index out of bounds");
                }

                val = " " + (lines + (insert_size/ROWS));

                for (count1 = 0; count1 < (index*ROWS); count1++) {
                        val = val + tokens.nextToken();
                }

                for (count1 = 0; count1 < insert_size; count1++) {
                        val = val + " " + value[count1];
                }

                for (count1 = (index*ROWS); count1 < (lines*ROWS); count1++) {
                        val = val + tokens.nextToken();
                }

                browser.newSendEvent(this, val);
	}
}
