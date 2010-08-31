package org.web3d.x3d.sai;

public interface MFInt32 extends MField {
	public void getValue(int[] values);
	public int get1Value(int index) throws ArrayIndexOutOfBoundsException;
	public void setValue(int size, int[] value);
	public void set1Value(int index, int value) throws ArrayIndexOutOfBoundsException;
	public void append(int[] value);
	public void insertValue(int index, int[] value);
}
