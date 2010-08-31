package org.web3d.x3d.sai;

public interface MField extends X3DField {
	public int size() throws InvalidFieldException, ConnectionException;
	public void clear() throws InvalidFieldException, ConnectionException;
	public void remove(int index) throws InvalidFieldException, ConnectionException, ArrayIndexOutOfBoundsException;
}
