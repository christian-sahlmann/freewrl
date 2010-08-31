package org.web3d.x3d.sai;

public interface MFTime extends MField {
	public void getValue(double[] value);
	public double get1Value(int index);
	public long get1JavaValue(int index);
	public void setValue(int size, double[] value);
	public void setValue(int size, long[] value);
	public void set1Value(int index, double value);
	public void set1Value(int index, long value);
	public void append(double value);
	public void append(long value);
	public void insertValue(int index, long value);
	public void insertValue(int index, double value);
}
