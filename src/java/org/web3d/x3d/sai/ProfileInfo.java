package org.web3d.x3d.sai;

public interface ProfileInfo {
	public String getName();
	public String getTitle();
	public ComponentInfo[] getComponents();
	public String toX3DString();
}
