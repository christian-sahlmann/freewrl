package sai;
import org.web3d.x3d.sai.*;

public class FreeWRLFieldDefinition implements X3DFieldDefinition {
	protected String name;
	protected int accessType;
	protected int fieldType;
	protected String fieldTypeString;
	protected String defaultVal;

	public FreeWRLFieldDefinition(String nm, int access, int field) {
		name = nm;
		accessType = access;
		fieldType = field;
		fieldTypeString = FreeWRLFieldTypes.getStringDesc(fieldType);
	}

	public String getName() {
		return name;
	}

	public int getAccessType() {
		return accessType;
	}

	public int getFieldType() {
		return fieldType;
	}

	public String getFieldTypeString() {
		return fieldTypeString;
	}

	public void setDefaultValue(String val) {
		defaultVal = val;
	}

	public String getDefault() {
		return defaultVal;
	}
}
