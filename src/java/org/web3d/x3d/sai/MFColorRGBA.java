package org.web3d.x3d.sai;

public interface MFColorRGBA extends MField {
	public void getValue(float[][] value);
	public void getValue(float[] value);
	public void get1Value(int index, float[] value);
	public void setValue(int numVolors, float[] value);
	public void setValue(int numColors, float[][] value);
	public void set1Value(int index, float[] value);
	public void append(float[] value);
	public void insertValue(int index, float[] value);
}
