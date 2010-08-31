package org.web3d.x3d.sai;

public interface X3DInterpolatorNode extends X3DChildNode {
	public void setFraction(float value);
	public int getNumKeys();
	public void setKey(float[] keys);
	public void getKey(float[] keys);
}
