package org.web3d.x3d.sai;

public interface MFString extends MField {
	public void getValue(String[] value);
	public String get1Value(int index);
	public void setValue(int numStrings, String[] value);
	public void set1Value(int index, String value);
	public void append(String[] value);
	public void insertValue(int index, String[] value);
}
