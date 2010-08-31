package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWMFNode extends FreeWRLMField implements MFNode {
	FreeWRLBrowser browser;
	
	FWMFNode(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public void getValue(X3DNode[] nodes) throws ArrayIndexOutOfBoundsException {
		String rep;
		StringTokenizer tokens;
		int count;
		int count2;

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			System.out.println("GOT REP: " + rep);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer(RLreturn);
		}

		count = tokens.countTokens();

		if (nodes.length < count) {
			throw new ArrayIndexOutOfBoundsException("MFNode getValue passed array of insufficient size");
		}
		
		count2 = 0;
		
		while (count2 < count) {
			nodes[count2] = new FreeWRLNode(browser);
			rep = tokens.nextToken();
			((FreeWRLNode)nodes[count2]).setPerlPtr(new String(rep));
			((FreeWRLNode)nodes[count2]).setPointer(new String(rep));
			count2++;
		}
	}
	public X3DNode get1Value(int index) throws ArrayIndexOutOfBoundsException {
		String rep;
		StringTokenizer tokens;
		int count;
		int count2;
		FreeWRLNode[] nodes;

		if (command != null) {
			rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
			tokens = new StringTokenizer(rep);
		} else {
			tokens = new StringTokenizer(RLreturn);
		}

		count = tokens.countTokens();

		if (index > count) {
			throw new ArrayIndexOutOfBoundsException("MFNode get1Value passed index out of bounds");
		}
		
		nodes = new FreeWRLNode[count];

		count2 = 0;
		
		while (count2 < count) {
			nodes[count2] = new FreeWRLNode(browser);
			rep = tokens.nextToken();
			nodes[count2].setPerlPtr(new String(rep));
			nodes[count2].setPointer(new String(rep));
			count2++;
		}
		return nodes[index];
	}
	public void setValue(int size, X3DNode[] value) {
		int count;

		if (size > value.length) {
			size = value.length;
		}

		for (count = 0; count < value.length; count++) {
			if ((((FreeWRLNode)(value[count])).getPointer()) == null) {
			}
			browser.SendChildEvent(nodePtr, offset, command, ((FreeWRLNode)value[count]).getPointer());
		}
	}
	public void set1Value(int index, X3DNode value) {
		browser.SendChildEvent(nodePtr, offset, command, ((FreeWRLNode)value).getPointer());
	}
	public void append(X3DNode value) {
               String rep;
                StringTokenizer tokens;
		String tempPtr, tempName;
                int count;
                int count2;

                if (command != null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                        tokens = new StringTokenizer(rep);
                } else {
                        tokens = new StringTokenizer(RLreturn);
                }

                count = tokens.countTokens();

                count2 = 0;

                while (count2 < count) {
                        rep = tokens.nextToken();
			tempName = new String(rep);
			rep = tokens.nextToken();
			tempPtr = new String(rep);
			browser.SendChildEvent(nodePtr, offset, command, tempPtr);
                        count2++;
                }
		
		browser.SendChildEvent(nodePtr, offset, command, ((FreeWRLNode)value).getPointer());
	}
	public void insertValue(int index, X3DNode value) {
               String rep;
                StringTokenizer tokens;
                int count;
                int count2;
		String tempPtr, tempName;

                if (command != null) {
                        rep = browser.SendEventOut(nodePtr, offset, datasize, dataType, command);
                        tokens = new StringTokenizer(rep);
                } else {
                        tokens = new StringTokenizer(RLreturn);
                }

                count = tokens.countTokens();

                count2 = 0;

                while (count2 < index) {
                        rep = tokens.nextToken();
                        tempName = new String(rep);
                        rep = tokens.nextToken();
                        tempPtr = new String(rep);
                        browser.SendChildEvent(nodePtr, offset, command, tempPtr);
                        count2++;
                }

                browser.SendChildEvent(nodePtr, offset, command, ((FreeWRLNode)value).getPointer());

                while (count2 < count) {
                        rep = tokens.nextToken();
                        tempName = new String(rep);
                        rep = tokens.nextToken();
                        tempPtr = new String(rep);
                        browser.SendChildEvent(nodePtr, offset, command, tempPtr);
                        count2++;
                }

	}
}
