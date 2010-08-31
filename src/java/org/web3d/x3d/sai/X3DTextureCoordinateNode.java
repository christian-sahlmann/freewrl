package org.web3d.x3d.sai;

public interface X3DTextureCoordinateNode extends X3DGeometricPropertyNode {
	public int getNumCoordinates();
	public int getNumComponents();
	public void setPoint(float[] points);
	public void getPoint(float[] points);
}
