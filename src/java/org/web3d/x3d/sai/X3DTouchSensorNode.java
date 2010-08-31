package org.web3d.x3d.sai;

public interface X3DTouchSensorNode extends X3DPointingDeviceSensorNode {
	public boolean getIsOver();
	public double getEnterTime();
	public double getTouchTime();
}
