package org.web3d.x3d.sai;

public interface X3DTimeDependentNode extends X3DChildNode {
	public boolean getIsActive();
	public boolean getIsPaused();
	public double getElapsedTime();
	public void setNumLoops(float count);
	public float getNumLoops();
	public void setLoop(boolean loop);
	public boolean getLoop();
	public void setStartTime(double time);
	public double getStartTime();
	public void setStopTime(double time);
	public double getStopTime();
	public void setPauseTime(double time);
	public double getPauseTime();
	public void setUnPauseTime(double time);
	public double getUnPauseTime();
}
