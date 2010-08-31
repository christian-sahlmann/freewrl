package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWProtoDeclaration implements X3DProtoDeclaration, X3DExternProtoDeclaration {
	String protoName;
	FreeWRLFieldDefinition[] fields;
	FreeWRLBrowser browser;
	int nodeType;

	FWProtoDeclaration(FreeWRLBrowser b) {
		browser = b;
	}
	public String getProtoName() {
		return protoName;
	}

	public String toString() {
		String temp;
		temp = "" + protoName + " " + FreeWRLFieldTypes.getStringDesc(nodeType);
		temp = temp + " " + fields.length; 
		for (int i = 0; i < fields.length; i++) {
			temp = temp + " " + fields[i].getFieldTypeString() + " " + fields[i].getName() + " " + fields[i].getDefault();
		}
		
		return temp;

	}
	public X3DProtoInstance createInstance() throws InvalidOperationTimingException, InvalidProtoException {
                String retval;
                StringTokenizer tokens;
                FWProtoInstance proto;

                retval = browser.sendGlobalCommand("b " + protoName);
                tokens = new StringTokenizer(retval);
                proto = new FWProtoInstance(browser);
                proto.setNodeName(tokens.nextToken());
                proto.setPointer(tokens.nextToken());

                return proto;
	}
	public X3DFieldDefinition[] getFieldDefinitions() throws InvalidOperationTimingException, InvalidProtoException {
		return fields;
	}

	public int getLoadState() {
		return -1;
	}

	public void loadNow() {
	}
	
	public void setProtoName(String name) {
		protoName = name;
	}

	public void setFields(FreeWRLFieldDefinition[] f) {
		fields = f;
	}

	public void setType(int t) {
		nodeType = t;
	}

	public int[] getNodeType() throws InvalidProtoException {
		int[] types;
		types = new int[1];
		types[0] = nodeType;

		return types;
	}

	public void dispose() {

	}
} 
