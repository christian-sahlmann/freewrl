package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWMFVec2d extends FreeWRLMField implements MFVec2d {
	FreeWRLBrowser browser;
	private static final int ROWS = 2;
	
	public FWMFVec2d(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(double[][] value) throws ArrayIndexOutOfBoundsException {
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
			throw new ArrayIndexOutOfBoundsException("MFVec2d getValue passed array of insufficient size");
		}

		for (count1=0; count1<lines; count1++) {
			if ((value[count1]).length < ROWS) {
				throw new ArrayIndexOutOfBoundsException("MFVec2d getValue passed array containing degenerate subarray value");
			}
			for (count2=0; count2<ROWS; count2++) {
				value[count1][count2] = Double.valueOf(tokens.nextToken()).doubleValue();
			}
		}
	}
	public void getValue(double[] value) throws ArrayIndexOutOfBoundsException {
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

		if (value.length < (lines*ROWS)) {
			throw new ArrayIndexOutOfBoundsException("MFVec2d getValue passed array of insufficient size");
		}

		for (count1=0; count1<(lines*ROWS); count1++) {
			value[count1] = Double.valueOf(tokens.nextToken()).doubleValue();
		}
	}
	public void get1Value(int index, double[] value) throws ArrayIndexOutOfBoundsException {
		double[][] tval;
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

		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("MFVec2d get1Value passed array of insufficient size");
		}
		
		if (index > lines) {
			throw new ArrayIndexOutOfBoundsException("MFVec2d get1Value passed index out of bounds");
		}

		tval = new double[lines][ROWS];

		for (count1=0; count1<lines; count1++) {
			for (count2=0; count2<ROWS; count2++) {
				tval[count1][count2] = Double.valueOf(tokens.nextToken()).doubleValue();
			}
		}

		for (count1=0; count1<ROWS; count1++) {
			value[count1] = tval[index][count1];
		}
	}
	public void setValue(int size, double[] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String val;

		if (value.length < (size*ROWS)) {
			throw new ArrayIndexOutOfBoundsException("MFVec2d setValue passed array containing an insufficient number of values");
		}

		val = " " + size;

		for (count = 0; count < (size*ROWS); count++) {
			val = val + " " + value[count];
		}
		browser.newSendEvent(this, val);
	}
	public void setValue(int size, double[][] value) throws ArrayIndexOutOfBoundsException {
		int count;
		String val;

		if (value.length < size) {
			size = value.length;
		}

		val = " " + size;

		for (count = 0; count < size; count++) {
			if ((value[count]).length < ROWS) {
				throw new ArrayIndexOutOfBoundsException("MFVec2d setValue degenerate vector value received");
			}
			val = val + " " + value[count][0] + " " + value[count][1];
		}
		browser.newSendEvent(this, val);
	}
	public void set1Value(int index, double[] value) throws ArrayIndexOutOfBoundsException {
		if (value.length < ROWS) {
			throw new ArrayIndexOutOfBoundsException("MFVec2d set1Value degenerate vector value received");
		}
		browser.newSendEvent(this, " ONEVAL " + index + " " + value[0] + " " + value[1]);
	}
	public void append(double[] value) {
                int lines;
                int count1;
                int count2;
                int count;
                StringTokenizer tokens;
                String rep;
                String val;
                int size;

                if (RLreturn == null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                } else {
                        rep = RLreturn;
                }

                tokens = new StringTokenizer(rep);

                lines = Integer.valueOf(tokens.nextToken()).intValue();
                size = value.length/ROWS;

                val = " " + (lines+size);

                for (count = 0; count < lines; count++) {
                        val = val + " " + Double.valueOf(tokens.nextToken()).doubleValue() + " " + Double.valueOf(tokens.nextToken()).doubleValue();
                }

                for (count = 0; count < value.length; count++) {
                        if (value.length < ROWS) {
                                throw new ArrayIndexOutOfBoundsException("MFVec2d append degenerate vector value received");
                        }

                        val = val + " " + value[count] + " " + value[count+1];
                        count++;
                }

                browser.newSendEvent(this, val);
	}
	public void insertValue(int index, double[] value) {
                int lines;
                int count1;
                int count2;
                int count;
                StringTokenizer tokens;
                String rep;
                String val;
                int size;

                if (RLreturn == null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                } else {
                        rep = RLreturn;
                }

                tokens = new StringTokenizer(rep);

                lines = Integer.valueOf(tokens.nextToken()).intValue();
                size = value.length/ROWS;

                val = " " + (lines+size);

                for (count = 0; count < index; count++) {
                        val = val + " " + Double.valueOf(tokens.nextToken()).doubleValue() + " " + Double.valueOf(tokens.nextToken()).doubleValue();
                }

                for (count = 0; count < value.length; count++) {
                        if (value.length < ROWS) {
                                throw new ArrayIndexOutOfBoundsException("MFVec2d insert degenerate vector value received");
                        }

                        val = val + " " + value[count] + " " + value[count+1];
                        count++;
                }

                for (count = (index+size); count < (lines+size); count++) {
                        val = val + " " + Double.valueOf(tokens.nextToken()).doubleValue() + " " + Double.valueOf(tokens.nextToken()).doubleValue();
                }

                browser.newSendEvent(this, val);
	}
}
