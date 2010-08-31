package org.web3d.x3d.sai;

public interface X3DNormalNode extends X3DGeometricPropertyNode {
	public int getNumNormals();
	public void setVector(float[] value);
	public void getVector(float[] value);
}
