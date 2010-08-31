package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWMFString extends FreeWRLMField implements MFString {
	FreeWRLBrowser browser;

	public FWMFString(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}
	
	public void getValue(String[] value) throws ArrayIndexOutOfBoundsException {
		String rep;
		StringTokenizer tokens;	
		int count;
	
		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep, "\"");
		} else {
			tokens = new StringTokenizer(RLreturn, "\"");
		}

		rep = "";

		if (value.length < (tokens.countTokens()/2)) {
			throw new ArrayIndexOutOfBoundsException("MFString getValue passed array of insufficient size");
		}

		count = 0;
		
		while(tokens.hasMoreTokens()) {
			value[count] = tokens.nextToken();
			if (value[count].equals("XyZZtitndi")) {
				value[count] = "";
			}
			if (tokens.hasMoreTokens()) rep = tokens.nextToken();
			count++;
		}
	}

	public String get1Value(int index) throws ArrayIndexOutOfBoundsException {
		String rep;
		String[] tstr;
		StringTokenizer tokens;	
		int count;
	
		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep, "\"");
		} else {
			tokens = new StringTokenizer(RLreturn, "\"");
		}

		rep = "";

		if (index > (tokens.countTokens()/2)) {
			throw new ArrayIndexOutOfBoundsException("MFString getValue passed index out of bounds");
		}

		count = 0;

		tstr = new String[(tokens.countTokens()/2)];
		
		while(tokens.hasMoreTokens()) {
			tstr[count] = tokens.nextToken();
			if (tstr[count].equals("XyZZtitndi")) {
				tstr[count] = "";
			}
			if (tokens.hasMoreTokens()) rep = tokens.nextToken();
			count++;
		}
		return tstr[index];
	}

	public void setValue(int numStrings, String[] value) {
		int count;
		String sestr;

		if (value.length < numStrings) {
			numStrings = value.length;
		}

		sestr = ""+numStrings+" ";
		for (count = 0; count < numStrings; count++) {
			sestr = sestr + " " + count + ";" + value[count].length() + ":"+value[count] + " ";
		}
		browser.newSendEvent(this, sestr);
	}

	public void set1Value(int index, String value) {
		browser.newSendEvent(this, ""+index+1+" "+index+";"+value.length()+":"+value+" ");
	}

	public void append(String[] value) {
		String rep;
		StringTokenizer tokens;	
		String sestr;
		int count;
		int numStrings;
		int numRx;
		int index;
		String tstr;
		String skip;
	
		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep, "\"");
		} else {
			tokens = new StringTokenizer(RLreturn, "\"");
		}

		numRx = tokens.countTokens()/2;
		numStrings = value.length;

		sestr = ""+(numStrings+numRx)+" ";
		
		index = 0;

		while(tokens.hasMoreTokens()) {
			tstr = tokens.nextToken();
			if (tstr.equals("XyZZtitndi")) {
				break;
			}
			sestr = sestr + " " + index+ ";" + tstr.length() + ":"+tstr + " ";
			index++;
			if (tokens.hasMoreTokens()) {
				skip = tokens.nextToken();
			}
		}

		for (count = 0; count < value.length; count++) {
			sestr = sestr + " " + (count+index) + ";" + value[count].length() + ":"+value[count] + " ";
		}
		browser.newSendEvent(this, sestr);

	}
	
	public void insertValue(int index, String[] value) {
                String rep;
                StringTokenizer tokens;
                String sestr;
                int count;
                int numInsert;
                int numRx;
		int numTotal;
                String tstr;
                String skip;

                if (command != null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                        tokens = new StringTokenizer(rep, "\"");
                } else {
                        tokens = new StringTokenizer(RLreturn, "\"");
                }

                numRx = tokens.countTokens()/2;
		numInsert = value.length;
		numTotal = numInsert + numRx;

                sestr = ""+numTotal+" ";

                count = 0;

                while((tokens.hasMoreTokens() && (count < index))) {
                        tstr = tokens.nextToken();
                        if (tstr.equals("XyZZtitndi")) {
                                break;
                        }
                        sestr = sestr + " " + count + ";" + tstr.length() + ":"+tstr + " ";
                        count++;
                        if (tokens.hasMoreTokens()) {
                                skip = tokens.nextToken();
                        }
                }
		
		for (int i = 0; i < numInsert; i++) {
			sestr = sestr + " " + count + ";" + value[i].length() + ":" + value[i] + " ";
			count++;
		}

                while((tokens.hasMoreTokens() && (count < numTotal))) {
                        tstr = tokens.nextToken();
                        if (tstr.equals("XyZZtitndi")) {
                                break;
                        }
                        sestr = sestr + " " + count + ";" + tstr.length() + ":"+tstr + " ";
                        count++;
                        if (tokens.hasMoreTokens()) {
                                skip = tokens.nextToken();
                        }
		}

                browser.newSendEvent(this, sestr);

	}
}
