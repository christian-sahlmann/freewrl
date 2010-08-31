package org.web3d.x3d.sai;

public interface X3DField {
	public X3DFieldDefinition getDefinition() throws InvalidFieldException, ConnectionException;
	public boolean isReadable() throws InvalidFieldException, ConnectionException;
	public boolean isWritable() throws InvalidFieldException, ConnectionException;
	public void addX3DEventListener(X3DFieldEventListener l) throws InvalidFieldException, ConnectionException;
	public void removeX3DEventListener(X3DFieldEventListener l) throws InvalidFieldException, ConnectionException;
	public void setUserData(Object data) throws InvalidFieldException, ConnectionException;
	public Object getUserData() throws InvalidFieldException, ConnectionException;
	public void dispose();
}
