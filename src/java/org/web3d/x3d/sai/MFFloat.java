package org.web3d.x3d.sai;

public interface MFFloat extends MField {
	public void getValue(float[] values);
	public float get1Value(int index) throws ArrayIndexOutOfBoundsException;
	public void setValue(int size, float[] value);
	public void set1Value(int index, float value) throws ArrayIndexOutOfBoundsException;
	public void append(float[] value);
	public void insertValue(int index, float[] value);
}
