package sai;
import org.web3d.x3d.sai.*;
import java.util.*;


public class FWMFColor extends FreeWRLMField implements MFColor {
	private static final int ROWS = 3;
	FreeWRLBrowser browser;

	public FWMFColor(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
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
			throw new ArrayIndexOutOfBoundsException("MFColor getValue passed array of insufficient size");
		}

		for (count1 = 0; count1<lines; count1++) {
			if (value[count1].length < ROWS) {
				throw new ArrayIndexOutOfBoundsException("MFColor getValue passed array of insufficient size");
			}
			for (count2 = 0; count2<ROWS; count2++) {
				value[count1][count2] = Float.valueOf(tokens.nextToken()).floatValue();
			}
		} 
	}
	public void getValue(float[] value) {
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
			throw new ArrayIndexOutOfBoundsException("MFColor getValue passed array of insufficient size");
		}
		for (count1 = 0; count1<(lines*ROWS); count1++) {
			value[count1] = Float.valueOf(tokens.nextToken()).floatValue();
		} 
	}
	public void get1Value(int index, float[] value) {
		int lines;
		float[][] tval;
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
		tval = new float[lines][3];
		for (count1 = 0; count1<lines; count1++) {
			for (count2 = 0; count2<ROWS; count2++) {
				tval[count1][count2] = Float.valueOf(tokens.nextToken()).floatValue();
			}
		} 

		value[0] = tval[index][0];
		value[1] = tval[index][1];
		value[2] = tval[index][2];
	}
	public void setValue(int numVals, float[] value) throws ArrayIndexOutOfBoundsException, IllegalArgumentException {
		int count;
		String val;

		val = " " + numVals;

		if (value.length < (numVals*ROWS)) {
			throw new ArrayIndexOutOfBoundsException("Colour value array contains insufficient number of values");
		}
		for (count = 0; count < (numVals*ROWS); count++) {
			if ((value[count] < 0) || (value[count] > 1)) {
				throw new IllegalArgumentException("Colour value out of bounds");
			}
			val = val + " " + value[count];
		}
		browser.newSendEvent(this, val);
	}
	public void setValue(int numVals, float[][] value) throws ArrayIndexOutOfBoundsException, IllegalArgumentException {
		int count;
		String val;

		val = " " + numVals;
		if (value.length < numVals) {
			throw new ArrayIndexOutOfBoundsException("Colour value array contains insufficient number of values");
		}
		for (count = 0; count < numVals; count++) {
			if ((value[count][0] < 0) || (value[count][0] > 1) || (value[count][1] < 0) || (value[count][1] > 1) || (value[count][2] < 0) || (value[count][2] > 1)) {
				throw new IllegalArgumentException("Colour value out of bounds");
			}
			if (value.length < ROWS) {
				throw new ArrayIndexOutOfBoundsException("Colour value array contains insufficient number of values");
			}
			val = val + " " + value[count][0] + " " + value[count][1] + " " + value[count][2];
		}
		browser.newSendEvent(this, val);
	}
	public void set1Value(int index, float[] value) throws IllegalArgumentException, ArrayIndexOutOfBoundsException {
		if ((value[0] < 0) || (value[0] > 1) || (value[1] < 0) || (value[1] > 1) || (value[2] < 0) || (value[2] > 1)) {
			throw new IllegalArgumentException("Colour value out of bounds");
		}
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("Colour value array contains insufficient number of values");
		}
		browser.newSendEvent(this, " ONEVAL " + index + " " + value[0] + " " + value[1] + " " + value[2]);
	}
	public void append(float[] value) throws IllegalArgumentException, ArrayIndexOutOfBoundsException {
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
			throw new ArrayIndexOutOfBoundsException("Color value array contains insufficient number of colors");
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
                        throw new ArrayIndexOutOfBoundsException("Color value array contains insufficient number of colors");
                }

                if ((index > (lines/ROWS)) || (index < 0)) {
                        throw new ArrayIndexOutOfBoundsException("MFColor insertValue passed index out of bounds");
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
