package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FreeWRLBrowserInfo {
	private static HashMap browserProperties;

	private static final int ABSTRACT_NODES = 0;
	private static final int CONCRETE_NODES = 1;
	private static final int EXTERNAL_INTERACTIONS = 2;
	private static final int PROTOTYPE_CREATE = 3;
	private static final int DOM_IMPORT = 4;
	private static final int XML_ENCODING = 5;
	private static final int CLASSIC_VRML_ENCODING = 6;
	private static final int BINARY_ENCODING = 7;

	static {
		browserProperties = new HashMap();
	
		browserProperties.put(new Integer(ABSTRACT_NODES), new Boolean(false));
		browserProperties.put(new Integer(CONCRETE_NODES), new Boolean(true));
		browserProperties.put(new Integer(EXTERNAL_INTERACTIONS), new Boolean(true));
		browserProperties.put(new Integer(PROTOTYPE_CREATE), new Boolean(true));
		browserProperties.put(new Integer(DOM_IMPORT), new Boolean(false));
		browserProperties.put(new Integer(XML_ENCODING), new Boolean(true));
		browserProperties.put(new Integer(CLASSIC_VRML_ENCODING), new Boolean(true));
		browserProperties.put(new Integer(BINARY_ENCODING), new Boolean(true));
	}

	public static void setBrowserProperty(int property, boolean value) {
		if ( property < 0 || property > 7) {
			System.out.println("Passed invalid property value.  Property set request ignored.");
		} else {
			browserProperties.remove(new Integer(property));
			browserProperties.put(new Integer(property), new Boolean(value));
		}
	}

	public static boolean getBrowserProperty(int property) {
		Boolean temp;
		temp = (Boolean) browserProperties.get(new Integer(property));
		return temp.booleanValue();
	}

	public static Map getBrowserProperties() {
		return browserProperties;
	}
}
