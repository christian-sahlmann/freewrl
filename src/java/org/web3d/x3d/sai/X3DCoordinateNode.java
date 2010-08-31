package org.web3d.x3d.sai;

public interface X3DCoordinateNode extends X3DGeometricPropertyNode {
	public int getNumCoordinates();
	public void setPoint(float[] points);
	public void getPoint(float[] points);
}
