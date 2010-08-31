package sai;
import org.web3d.x3d.sai.*;

public class FWProfileInfo implements ProfileInfo {
	private String name;
	private String title;
	private ComponentInfo[] components;

	public FWProfileInfo(String n, String t, ComponentInfo[] c) {
		name = n;
		title = t;
		components = c;
	}

	public String getName() {
		return name;
	}

	public String getTitle() {
		return title;
	}
	
	public ComponentInfo[] getComponents() {
		return components;
	}

	public String toX3DString() {
		return "PROFILE " + name;
	}
}
