package org.web3d.x3d.sai;

public interface X3DSequencerNode extends X3DChildNode {
	public void setFraction(float fraction);
	public int getNumKey();
	public void getKey(float[] keys);
	public void setKey(float[] keys);
	public int getNumKeyValue();
}
