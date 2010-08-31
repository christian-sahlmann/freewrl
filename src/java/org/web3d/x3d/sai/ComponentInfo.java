package org.web3d.x3d.sai;

public interface ComponentInfo {
	public String getName();
	public int getLevel();
	public String getTitle();
	public String getProviderURL();
	public String toX3DString();
}
