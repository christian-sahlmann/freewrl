package org.web3d.x3d.sai;

public interface X3DSoundSourceNode {
	public float getPitch();
	public void setPitch(float pitch) throws InvalidFieldValueException;
	public void setDescription(String text);
	public String getDescription(String text);
}
