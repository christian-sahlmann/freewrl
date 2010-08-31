package org.web3d.x3d.sai;

public interface X3DColorNode extends X3DGeometricPropertyNode {
	public int getNumColors();
	public int getNumComponents();
	public void setColor(float[] colors);
	public void getColor(float[] color);
}
