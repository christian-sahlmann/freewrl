package sai;
import org.web3d.x3d.sai.*;

public class FreeWRLField implements X3DField {
	String RLreturn;
	String command;
	String node;
	String dataType;
	String nodePtr;
	String offset;
	String datasize;
	String scripttype;
	boolean disposed;

	protected FreeWRLFieldDefinition fieldDef;
	protected Object userData;
	protected FreeWRLBrowser browser;

	public FreeWRLField(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		fieldDef = def;
		userData = null;
		browser = b;
		disposed = false;
	}
	
	public String toString() {
		String str;
		str = new String("Field with command " + command + " for node " + node);
		return str;
	}

	public X3DFieldDefinition getDefinition() throws InvalidFieldException, ConnectionException{
		checkValid();
		return fieldDef;
	}
	public boolean isReadable() throws InvalidFieldException, ConnectionException {
		checkValid();
		int type = fieldDef.getAccessType();
		if ((type == FreeWRLFieldTypes.INPUT_OUTPUT) || (type == FreeWRLFieldTypes.OUTPUT_ONLY))
			return true;
		else
			return false;
	}
	public boolean isWritable() throws InvalidFieldException, ConnectionException {
		checkValid();
		int type = fieldDef.getAccessType();
		if ((type == FreeWRLFieldTypes.INPUT_OUTPUT) || (type == FreeWRLFieldTypes.INPUT_ONLY))
			return true;
		else
			return false;
	}
	public void addX3DEventListener (X3DFieldEventListener l) throws ConnectionException, InvalidFieldException {
		int evType;
		checkValid();
		evType = fieldDef.getFieldType();
		browser.RegisterListener(l, (Object) userData, nodePtr, offset, dataType,  datasize, evType); 
	}

	public void removeX3DEventListener(X3DFieldEventListener l) throws ConnectionException, InvalidFieldException {
		int evType;
		checkValid();
		evType = fieldDef.getFieldType();
		browser.unRegisterListener(l, nodePtr, offset, dataType, datasize, evType); 
	}

	public void setUserData(Object data) throws InvalidFieldException, ConnectionException {
		checkValid();
		userData = data;
	}

	public Object getUserData() throws InvalidFieldException, ConnectionException {
		checkValid();
		return userData;
	}

	public void dispose() {
		disposed = true;
	}

	public void checkValid() {
		if (disposed) {
			throw new InvalidFieldException("This field has been disposed");
		}
	}

	public void setCommand(String com) {
		command = com;
	}

	public void setNode(String nod) {
		node = nod;
	}

	public void setDataType (String dt) {
		dataType = dt;
	}

	public void setNodePtr(String np) {
		nodePtr = np;
	}

	public void setOffset(String off) {
		offset = off;
	}

	public void setDataSize(String ds) {
		datasize = ds;
	}
	
	public void setScriptType(String st) {
		scripttype = st;
	}

	public String getDataSize() {
		return datasize;
	}

	public String getScriptType() {
		return scripttype;
	}

	public String getCommand() {
		return command;
	}
	
	public String getNode() {
		return node;
	}

	public String getDataType() {
		return dataType;
	}
	
	public String getNodePtr() {
		return nodePtr;
	}

	public String getOffset() {
		return offset;
	}
}
