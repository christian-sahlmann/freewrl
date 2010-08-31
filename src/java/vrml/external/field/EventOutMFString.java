package vrml.external.field;
import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;


public class EventOutMFString extends EventOutMField {
	// retstr is an array of string values.
	// mySize is the size of retstr.

	String[] retstr;
	int mySize = 0;
	String[] spl;

	public EventOutMFString() {EventType = FieldTypes.MFSTRING;}

	public String[] getValue() {
		String rep;

		if (command != null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
			// System.out.println ("rep is " + rep);
			spl = rep.split("\"",0);
		} else {
			spl = RLreturn.split("\"",0);
		}

		/* 
		System.out.println ("token count is " +
			(tokens.countTokens());

		System.out.println ("split len is " + spl.length);

		for (int i = 0; i < spl.length; i++) {
			System.out.println ("element " + i + " is " + spl[i]);
		}
		*/
		/*
		rep is "kiwano.wrl" "TinMan.wrl" "Angel.wrl" "halfmoon.wrl" ""  
		token count is 4
		split len is 11
		element 0 is 
		element 1 is kiwano.wrl
		element 2 is  
		element 3 is TinMan.wrl
		element 4 is  
		element 5 is Angel.wrl
		element 6 is  
		element 7 is halfmoon.wrl
		element 8 is  
		element 9 is 
		*/

		mySize = spl.length;
		retstr = new String[mySize];
		//System.out.println ("sizeof is " + mySize);
		for (int i=1; i<spl.length; i++) {
			retstr[i] = spl[i];
			//System.out.println ("string " + i + " value: " + retstr[i]);
		}
		return retstr;
	}

	public String get1Value(int index) {
		if ((index > mySize) || (index < 0)) {
			System.out.println ("EventOutMFString.get1Value - index " + index +
			" out of range");
			index = 0;
		}
		return retstr[index];
	}
}
