package org.web3d.x3d.sai;

public interface X3DNode {
	public void setMetadata(X3DMetadataObject data) throws InvalidNodeException, ConnectionException;
	public X3DMetadataObject getMetadata() throws InvalidNodeException, ConnectionException;
	public String getNodeName() throws InvalidNodeException, ConnectionException;
	public X3DFieldDefinition[] getFieldDefinitions() throws InvalidNodeException, ConnectionException;
	public int[] getNodeType() throws InvalidNodeException, ConnectionException;
	public X3DField getField(String name) throws InvalidNameException, InvalidNodeException, ConnectionException;
	public void dispose();
}
