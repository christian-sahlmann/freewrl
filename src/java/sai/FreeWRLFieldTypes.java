package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FreeWRLFieldTypes implements X3DFieldTypes {
	private static final HashMap fieldTypes;
	private static final HashMap fieldTypesb;
	private static final HashMap fieldTypesc;
	private static final HashMap fieldTypesd;
	private static final HashMap accessTypes;
	private static final HashMap accessTypesb;
	private static final HashMap accessTypesc;
	
	public static int SFUNKOWN = 0;

	static {
		fieldTypes = new HashMap();

		fieldTypes.put("a", new Integer(SFUNKOWN)); 
		fieldTypes.put("b", new Integer (SFBOOL)); 
		fieldTypes.put("c", new Integer (SFCOLOR)); 
		fieldTypes.put("d", new Integer (SFFLOAT)); 
		fieldTypes.put("e", new Integer (SFTIME)); 
		fieldTypes.put("f", new Integer (SFINT32)); 
		fieldTypes.put("g", new Integer (SFSTRING)); 
		fieldTypes.put("h", new Integer (SFNODE)); 
		fieldTypes.put("i", new Integer (SFROTATION)); 
		fieldTypes.put("j", new Integer (SFVEC2F)); 
		fieldTypes.put("k", new Integer (SFIMAGE)); 
		fieldTypes.put("l", new Integer (MFCOLOR)); 
		fieldTypes.put("m", new Integer (MFFLOAT)); 
		fieldTypes.put("n", new Integer (MFTIME)); 
		fieldTypes.put("o", new Integer (MFINT32)); 
		fieldTypes.put("p", new Integer (MFSTRING)); 
		fieldTypes.put("q", new Integer (MFNODE)); 
		fieldTypes.put("r", new Integer (MFROTATION)); 
		fieldTypes.put("s", new Integer (MFVEC2F)); 
		fieldTypes.put("t", new Integer (MFVEC3F)); 
		fieldTypes.put("u", new Integer (SFVEC3F)); 
		fieldTypes.put("v", new Integer (SFCOLORRGBA)); 
		fieldTypes.put("w", new Integer (MFCOLORRGBA)); 
		fieldTypes.put("x", new Integer (SFDOUBLE)); 
		fieldTypes.put("y", new Integer (MFDOUBLE)); 
		fieldTypes.put("z", new Integer (SFVEC3D)); 
		fieldTypes.put("A", new Integer (MFVEC3D)); 
		fieldTypes.put("B", new Integer (SFVEC2D)); 
		fieldTypes.put("C", new Integer (MFVEC2D)); 

		fieldTypesb = new HashMap();

		fieldTypesb.put(new Integer(SFUNKOWN), "a");
		fieldTypesb.put(new Integer(SFBOOL), "b");
		fieldTypesb.put(new Integer(SFCOLOR), "c");
		fieldTypesb.put(new Integer(SFFLOAT), "d");
		fieldTypesb.put(new Integer(SFTIME), "e");
		fieldTypesb.put(new Integer(SFINT32), "f");
		fieldTypesb.put(new Integer(SFSTRING), "g");
		fieldTypesb.put(new Integer(SFNODE), "h");
		fieldTypesb.put(new Integer(SFROTATION), "i");
		fieldTypesb.put(new Integer(SFVEC2F), "j");
		fieldTypesb.put(new Integer(SFIMAGE), "k");
		fieldTypesb.put(new Integer(MFCOLOR), "l");
		fieldTypesb.put(new Integer(MFFLOAT), "m");
		fieldTypesb.put(new Integer(MFTIME), "n");
		fieldTypesb.put(new Integer(MFINT32), "o");
		fieldTypesb.put(new Integer(MFSTRING), "p");
		fieldTypesb.put(new Integer(MFNODE), "q");
		fieldTypesb.put(new Integer(MFROTATION), "r");
		fieldTypesb.put(new Integer(MFVEC2F), "s");
		fieldTypesb.put(new Integer(MFVEC3F), "t");
		fieldTypesb.put(new Integer(SFVEC3F), "u");
		fieldTypesb.put(new Integer(SFCOLORRGBA), "v");
		fieldTypesb.put(new Integer(MFCOLORRGBA), "w");
		fieldTypesb.put(new Integer(SFDOUBLE), "x");
		fieldTypesb.put(new Integer(MFDOUBLE), "y");
		fieldTypesb.put(new Integer(SFVEC3D), "z");
		fieldTypesb.put(new Integer(MFVEC3D), "A");
		fieldTypesb.put(new Integer(SFVEC2D), "B");
		fieldTypesb.put(new Integer(MFVEC2D), "C");

		fieldTypesc = new HashMap();

		fieldTypesc.put(new Integer(SFUNKOWN), "SFUnknown");
		fieldTypesc.put(new Integer(SFBOOL), "SFBool");
		fieldTypesc.put(new Integer(SFCOLOR), "SFColor");
		fieldTypesc.put(new Integer(SFFLOAT), "SFFloat");
		fieldTypesc.put(new Integer(SFTIME), "SFTime");
		fieldTypesc.put(new Integer(SFINT32), "SFInt32");
		fieldTypesc.put(new Integer(SFSTRING), "SFString");
		fieldTypesc.put(new Integer(SFNODE), "SFNode");
		fieldTypesc.put(new Integer(SFROTATION), "SFRotation");
		fieldTypesc.put(new Integer(SFVEC2F), "SFVec2f");
		fieldTypesc.put(new Integer(SFIMAGE), "SFImage");
		fieldTypesc.put(new Integer(MFCOLOR), "MFColor");
		fieldTypesc.put(new Integer(MFFLOAT), "MFFloat");
		fieldTypesc.put(new Integer(MFTIME), "MFTime");
		fieldTypesc.put(new Integer(MFINT32), "MFInt32");
		fieldTypesc.put(new Integer(MFSTRING), "MFString");
		fieldTypesc.put(new Integer(MFNODE), "MFNode");
		fieldTypesc.put(new Integer(MFROTATION), "MFRotation");
		fieldTypesc.put(new Integer(MFVEC2F), "MFVec2f");
		fieldTypesc.put(new Integer(MFVEC3F), "MFVec3f");
		fieldTypesc.put(new Integer(SFVEC3F), "SFVec3f");
		fieldTypesc.put(new Integer(SFCOLORRGBA), "SFColorRGBA");
		fieldTypesc.put(new Integer(MFCOLORRGBA), "MFColorRGBA");
		fieldTypesc.put(new Integer(SFDOUBLE), "SFDouble");
		fieldTypesc.put(new Integer(MFDOUBLE), "MFDouble");
		fieldTypesc.put(new Integer(SFVEC3D), "SFVec3d");
		fieldTypesc.put(new Integer(MFVEC3D), "MFVec3d");
		fieldTypesc.put(new Integer(SFVEC2D), "SFVec2d");
		fieldTypesc.put(new Integer(MFVEC2D), "MFVec2d");


		fieldTypesd = new HashMap();

		fieldTypesd.put("SFUnkown", new Integer(SFUNKOWN));
		fieldTypesd.put("SFBool", new Integer(SFBOOL));
		fieldTypesd.put("SFColor", new Integer(SFCOLOR));
		fieldTypesd.put("SFFloat", new Integer(SFFLOAT));
		fieldTypesd.put("SFTime", new Integer(SFTIME));
		fieldTypesd.put("SFInt32", new Integer(SFINT32));
		fieldTypesd.put("SFString", new Integer(SFSTRING));
		fieldTypesd.put("SFNode", new Integer(SFNODE));
		fieldTypesd.put("SFRotation", new Integer(SFROTATION));
		fieldTypesd.put("SFVec2f", new Integer(SFVEC2F));
		fieldTypesd.put("SFImage", new Integer(SFIMAGE));
		fieldTypesd.put("MFColor", new Integer(MFCOLOR));
		fieldTypesd.put("MFFloat", new Integer(MFFLOAT));
		fieldTypesd.put("MFTime", new Integer(MFTIME));
		fieldTypesd.put("MFInt32", new Integer(MFINT32));
		fieldTypesd.put("MFString", new Integer(MFSTRING));
		fieldTypesd.put("MFNode", new Integer(MFNODE));
		fieldTypesd.put("MFRotation", new Integer(MFROTATION));
		fieldTypesd.put("MFVec2f", new Integer(MFVEC2F));
		fieldTypesd.put("MFVec3f", new Integer(MFVEC3F));
		fieldTypesd.put("SFVec3f", new Integer(SFVEC3F));
		fieldTypesd.put("SFColorRGBA", new Integer(SFCOLORRGBA));
		fieldTypesd.put("MFColorRGBA", new Integer(MFCOLORRGBA));
		fieldTypesd.put("SFDouble", new Integer(SFDOUBLE));
		fieldTypesd.put("MFDouble", new Integer(MFDOUBLE));
		fieldTypesd.put("SFVec3d", new Integer(SFVEC3D));
		fieldTypesd.put("MFVec3d", new Integer(MFVEC3D));
		fieldTypesd.put("SFVec2d", new Integer(SFVEC2D));
		fieldTypesd.put("MFvec2d", new Integer(MFVEC2D));

		accessTypes = new HashMap();
		accessTypes.put("INPUT_ONLY", new Integer(INPUT_ONLY));
		accessTypes.put("INITIALIZE_ONLY", new Integer(INITIALIZE_ONLY));
		accessTypes.put("INPUT_OUTPUT", new Integer(INPUT_OUTPUT));
		accessTypes.put("OUTPUT_ONLY", new Integer(OUTPUT_ONLY));

		accessTypesc = new HashMap();
		accessTypesc.put("eventIn", new Integer(INPUT_ONLY));
		accessTypesc.put("field", new Integer(INITIALIZE_ONLY));
		accessTypesc.put("exposedField", new Integer(INPUT_OUTPUT));
		accessTypesc.put("eventOut", new Integer(OUTPUT_ONLY));

		accessTypesb = new HashMap();
		accessTypesb.put(new Integer(INPUT_ONLY), "INPUT_ONLY");
		accessTypesb.put(new Integer(INITIALIZE_ONLY), "INITIALIZE_ONLY");
		accessTypesb.put(new Integer(INPUT_OUTPUT), new Integer(INPUT_OUTPUT));
		accessTypesb.put(new Integer(OUTPUT_ONLY), new Integer(OUTPUT_ONLY));
	}

	public static int getIntType(String type) {
		Integer temp;
		temp = (Integer) fieldTypes.get(type);
		return temp.intValue();
	}

	public static String getStringType(int type) {
		return (String) fieldTypesb.get(new Integer(type));
	}

	public static String getStringDesc(int type) {
		return (String) fieldTypesc.get(new Integer(type));
	}

	public static int getIntFromStringDesc(String desc) {
		Integer temp;
		temp = (Integer) fieldTypesd.get(desc);
		return temp.intValue();
	}

	public static int getAccessFromType(String type) {
		Integer temp;
		temp = (Integer) accessTypesc.get(type);
		return temp.intValue();
	}

	public static int getIntAccess(String type) {
		Integer temp;
		temp = (Integer) accessTypes.get(type);
		return temp.intValue();
	}

	public static String getStringAccess(int type) {
		return (String) accessTypesb.get(new Integer(type));
	} 
}
	
