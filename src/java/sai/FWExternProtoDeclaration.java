package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWExternProtoDeclaration implements X3DExternProtoDeclaration{
	String protoName;
	FreeWRLFieldDefinition[] fields;
	FreeWRLBrowser browser;
	int nodeType;

	FWExternProtoDeclaration(FreeWRLBrowser b) {
		browser = b;
	}
	public String getProtoName() {
		return protoName;
	}
	public int getLoadState() {
		return 0;
	}
	public void loadNow() {
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
	
	public void setProtoName(String name) {
		protoName = name;
	}

	public void setFields(FreeWRLFieldDefinition[] f) {
		fields = f;
	}

	public void setType(int t) {
		nodeType = t;
	}

	public void dispose() {

	}
} 
