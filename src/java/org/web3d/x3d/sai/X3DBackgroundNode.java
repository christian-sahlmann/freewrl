package org.web3d.x3d.sai;

public interface X3DBackgroundNode extends X3DBindableNode {
	public int getNumSkyAngle();
	public void getSkyAngle(float[] angles);
	public void setSkyAngle(float[] angles);
	public int getNumGroundAngle();
	public void getGroundAngle(float[] angle);
	public void setGroundAngle(float[] angle);
	public int getNumSkyColor();
	public void getSkyColor(float[] colors);
	public void setSkyColor(float[] colors);
	public int getNumGroundColor();
	public void getGroundColor(float[] color);
	public void setGroundColor(float[] color);
}
