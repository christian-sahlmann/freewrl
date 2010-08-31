package org.web3d.x3d.sai;

public interface MFDouble extends MField {
	public void getValue(double[] values);	
	public double get1Value(int index) throws ArrayIndexOutOfBoundsException;
	public void setValue(int size, double[] value);
	public void set1Value(int index, double value) throws ArrayIndexOutOfBoundsException;
	public void append(double[] value);
	public void insertValue(int index, double[] value);
}
