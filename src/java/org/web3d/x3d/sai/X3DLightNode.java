package org.web3d.x3d.sai;

public interface X3DLightNode extends X3DChildNode {
	public boolean getOn();
	public void setOn(boolean state);
	public float getAmbientIntensity();
	public void setAmbientIntensity(float intensity) throws InvalidFieldValueException;
	public void getColor(float[] color);
	public void setColor(float[] color) throws InvalidFieldValueException;
	public void getIntensity();
	public void setIntensity(float newIntensity) throws InvalidFieldValueException;
}
