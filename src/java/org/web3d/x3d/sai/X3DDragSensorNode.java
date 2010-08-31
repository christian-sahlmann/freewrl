package org.web3d.x3d.sai;

public interface X3DDragSensorNode extends X3DPointingDeviceSensorNode {
	public void setAutoOffset(boolean newAutoOffset);
	public boolean getAutoOffset();
	public void getTrackPoint(float[] points);
}	
