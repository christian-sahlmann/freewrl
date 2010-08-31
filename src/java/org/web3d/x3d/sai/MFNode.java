package org.web3d.x3d.sai;

public interface MFNode extends MField {
	public void getValue(X3DNode[] nodes);
	public X3DNode get1Value(int index);
	public void setValue(int size, X3DNode[] value);
	public void set1Value(int index, X3DNode value);
	public void append(X3DNode value);
	public void insertValue(int index, X3DNode value);
}
