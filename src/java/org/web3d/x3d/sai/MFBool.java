package org.web3d.x3d.sai;

public interface MFBool extends MField {
	public void getValue(boolean[] vals);
	public boolean get1Value(int index);
	public void setValue(int size, boolean[] value);
	public void set1Value(int index, boolean value) throws ArrayIndexOutOfBoundsException;
	public void append(boolean value);
	public void insertValue(int index, boolean value);
}
