package org.web3d.x3d.sai;

public interface X3DAudioClipNode extends X3DTimeDependentNode, X3DUrlObject {
	public String getDescription();
	public void setDescription();
	public float getPitch();
	public void setPitch(float pitch) throws InvalidFieldValueException;
	public double getDuration();
	public void setDuration(double time);
}
