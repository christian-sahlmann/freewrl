package org.web3d.x3d.sai;

public interface MFVec2d extends MField {
	public void getValue(double[][] value);
	public void getValue(double[] value);
	public void get1Value(int index, double[] value);
	public void setValue(int size, double[] value);
	public void setValue(int size, double[][] value);
	public void set1Value(int index, double[] value);
	public void append(double[] value);
	public void insertValue(int index, double[] value);
}
