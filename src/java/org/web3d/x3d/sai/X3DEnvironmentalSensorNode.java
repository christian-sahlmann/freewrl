package org.web3d.x3d.sai;

public interface X3DEnvironmentalSensorNode extends X3DSensorNode {
	public double getEnterTime();
	public double getExitTime();
	public void getCenter(float[] pos);
	public void setCenter(float[] pos);
	public void getSize(float[] size);
	public void setSize(float[] size);
}
