package sai;
import org.web3d.x3d.sai.*;

import java.util.*;

public class FreeWRLNode implements X3DNode {
	private String name = null; // Node name
	private int type = -1;  // Node type
	private String ptr = null;  // Pointer to the node
	private String perlPtr = null;
	boolean disposed;
	FreeWRLBrowser browser;

	public FreeWRLNode(FreeWRLBrowser b) {
		browser = b;
		disposed = false;
	}

	public String toString() {
		String str;
		str = new String("NODE" + perlPtr);
		return str;
	}

	public boolean equals(Object o) {
		return (o != null) && (o instanceof FreeWRLNode) && (ptr== ((FreeWRLNode)o).getPointer()); 
	}

	public String getNodeName() throws InvalidNodeException, ConnectionException  {
		checkValid();
		return name;
	}

	public void setPerlPtr(String p) {
		perlPtr = p;
	}

	public String getPerlPtr() {
		return perlPtr;
	}

	public String getName() {
		return name;
	}
	public int[] getNodeType() throws InvalidNodeException, ConnectionException  {
		checkValid();
		String retval;
		int[] ret = new int[1];
		retval = browser.sendGlobalCommand("k " + ptr);
		ret[0] = Integer.parseInt(retval);
		return ret;
	}
	public X3DFieldDefinition[] getFieldDefinitions() throws InvalidNodeException, ConnectionException  {
		String command;
		String retval;
		String name, access, type;
		StringTokenizer tokens;
		FreeWRLFieldDefinition[] defs;
		int numFields;
		int i;
		int iaccess, itype;

		checkValid();
		command = "h " + ptr;
		retval = browser.sendGlobalCommand(command);
		System.out.println("got retval: " + retval);

		tokens= new StringTokenizer(retval);

		if ((retval == null) || (retval.equals(""))) {
			return null;
		}

		numFields = Integer.parseInt(tokens.nextToken());

		defs = new FreeWRLFieldDefinition[numFields];

		for (i = 0; i < numFields; i++) {
			name = tokens.nextToken();
			type = tokens.nextToken();
			access = tokens.nextToken();
			itype = FreeWRLFieldTypes.getIntType(type);
			iaccess = FreeWRLFieldTypes.getAccessFromType(access);
			defs[i] = new FreeWRLFieldDefinition(name, iaccess, itype);
		}
		
		return defs;
	}
	public X3DField getField(String fieldName) throws InvalidNameException, InvalidNodeException, ConnectionException {
		FreeWRLField ret;
		String NNN = "nodeFrom_getEventIn";
		StringTokenizer tokens;

		checkValid();
		
		// Return the type that is asked for.  To determine the subclass, look at the string.
		String st = browser.SendEventType(perlPtr, ptr, fieldName, "eventIn");
		
		tokens = new StringTokenizer(st);
		String NNPR = tokens.nextToken();
		String NOFF = tokens.nextToken();
		String NDS = tokens.nextToken();
		String NewDT = tokens.nextToken();
		String ScrT = tokens.nextToken();
		String access = tokens.nextToken();

		if (access.equals("FreeWRLPTR")) {
			throw new InvalidFieldException("Field name " + fieldName + " is not a valid field for this node");
		}
		int ia = FreeWRLFieldTypes.getAccessFromType(access);
		FreeWRLFieldDefinition def;

		if (NewDT.equals("p")) { 
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFString(def, browser);
		} else if (NewDT.equals("k")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFImage(def, browser);
		} else if (NewDT.equals("e")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFTime(def, browser);
		} else if (NewDT.equals("c")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFColor(def, browser);
		} else if (NewDT.equals("l")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFColor(def, browser);
		} else if (NewDT.equals("d")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFFloat(def, browser);
		} else if (NewDT.equals("m")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFFloat(def, browser);
		} else if (NewDT.equals("o")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFInt32(def, browser);
		} else if (NewDT.equals("h")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFNode(def, browser);
		} else if (NewDT.equals("r")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFRotation(def, browser);
		} else if (NewDT.equals("s")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFVec2f(def, browser);
		} else if (NewDT.equals("j")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFVec2f(def, browser);
		} else if (NewDT.equals("l")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFVec3f(def, browser);
		} else if (NewDT.equals("q")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFNode(def, browser);
		} else if (NewDT.equals("i")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFRotation(def, browser);
		} else if (NewDT.equals("g")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFString(def, browser);
		} else if (NewDT.equals("b")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFBool(def, browser);
		} else if (NewDT.equals("f")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFInt32(def, browser);
		} else if (NewDT.equals("v")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFColorRGBA(def, browser);
		} else if (NewDT.equals("w")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFColorRGBA(def, browser);
		} else if (NewDT.equals("u")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFVec3f(def, browser);
		} else if (NewDT.equals("z")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFVec3d(def, browser);
		} else if (NewDT.equals("A")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFVec3d(def, browser);
		} else if (NewDT.equals("B")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWSFVec2d(def, browser);
		} else if (NewDT.equals("C")) {
			def = new FreeWRLFieldDefinition(fieldName, ia, FreeWRLFieldTypes.getIntType(NewDT));
			ret = new FWMFVec2d(def, browser);
		} else {
			throw new InvalidFieldException("Field name " + fieldName + " is not a valid field for this node");
		} 
				
		ret.setCommand(fieldName);
		ret.setNode(NNN);
		ret.setDataType(NewDT);
		ret.setNodePtr(NNPR);
		ret.setOffset(NOFF);
		ret.setDataSize(NDS);
		ret.setScriptType(ScrT);
		return ret;
	}
	public void dispose() throws InvalidNodeException {
		disposed = true;
	}
	
	public void setNodeName(String n) {
		name = n;
	}

	public void setType(int t) {
		type = t;
	}

	public void setPointer(String p) {
		ptr = p;
	}

	public String getPointer() {
		return ptr;
	}

	public void setMetadata(X3DMetadataObject data) throws InvalidNodeException, ConnectionException  {
		checkValid();
	}

	public X3DMetadataObject getMetadata() throws InvalidNodeException, ConnectionException  {
		checkValid();
		return null;
	}

	private void checkValid() {
		if (disposed) {
			throw new InvalidNodeException ("This node has been disposed");
		}
	}
}
