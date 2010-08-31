package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FreeWRLMField extends FreeWRLField implements MField {
	FreeWRLBrowser browser;
	public FreeWRLMField(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}
	public int size() throws InvalidFieldException, ConnectionException {
		int lines;
		StringTokenizer tokens;
		String rep;

		checkValid();
		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		tokens = new StringTokenizer(rep);
		if (dataType.equals("q")) {
			lines = tokens.countTokens();;
		} else {
			lines = Integer.valueOf(tokens.nextToken()).intValue();
		}
		return lines;
	}
	public void clear() throws InvalidFieldException, ConnectionException {
		String val;
		checkValid();
		val = " 0";
		browser.newSendEvent(this, val);
	}
	public void remove(int index) throws InvalidFieldException, ConnectionException, ArrayIndexOutOfBoundsException { 
		int lines;
		int count1, count2;
		int size;
		StringTokenizer tokens;
		String rep;
		String val;

		checkValid();

		if (RLreturn == null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
		} else {
			rep = RLreturn;
		}

		System.out.println("got rep: " + rep);

		tokens = new StringTokenizer(rep);
		if (this instanceof MFNode) {
			lines = tokens.countTokens();
		} else {
			lines = Integer.valueOf(tokens.nextToken()).intValue();
		}
		
		if ((index > lines) || (index < 0)) {
			throw new ArrayIndexOutOfBoundsException("MFField remove passed index out of bounds");
		}
		
		lines--;

		val = " " + lines;

		if (this instanceof MFNode) {
			clear();
			size = tokens.countTokens();

			for (count1 = 0; count1 < lines; count1++) {
				for (count2 = 0; count2 < size; count2++) {
					if (count1 != index) {
						browser.SendChildEvent(nodePtr, offset, command,tokens.nextToken());
					 } else {
						tokens.nextToken();
					}
				}
			}
		} else {

			size = tokens.countTokens() / lines;
			for (count1 = 0; count1 < lines; count1++) {
				for (count2 = 0; count2 < size; count2++) {
					if (count1 != index) {
						val = val + tokens.nextToken();
					} else {
						tokens.nextToken();
					}
				}
			}
			browser.newSendEvent(this, val);
		}	
	}
}
