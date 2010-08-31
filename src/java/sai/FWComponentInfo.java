package sai;
import org.web3d.x3d.sai.*;

public class FWComponentInfo implements ComponentInfo {
	private String name;
	private int level;
	private String title;
	private String url;
	private String x3dstring;

	public FWComponentInfo(String n, int l, String t, String u) {
		name = n;
		level = l;
		title = t;
		url = u;
	}

	public String getName() {
		return name;
	}

	public int getLevel() {
		return level;
	}

	public String getTitle() {
		return title;
	}

	public String getProviderURL() {
		return url;
	}

	public String toX3DString() {
		return "COMPONENT " + name + ':' + level;
	}
	
}
		
