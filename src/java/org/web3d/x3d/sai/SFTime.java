package org.web3d.x3d.sai;

public interface SFTime extends X3DField {
	public double getValue();
	public long getJavaValue();
	public void setValue(double value);
	public void setValue(long value);
}
