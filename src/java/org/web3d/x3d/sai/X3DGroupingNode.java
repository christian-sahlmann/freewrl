package org.web3d.x3d.sai;

public interface X3DGroupingNode extends X3DChildNode, X3DBoundedObject {
	public void getChildren(X3DNode[] nodes);
	public void setChildren(X3DNode[] kids) throws InvalidNodeException;
	public void addChildren(X3DNode[] added) throws InvalidNodeException;
	public void removeChildren(X3DNode[] removed) throws InvalidNodeException;
	public void removeChild(X3DNode removed) throws InvalidNodeException;
	public int getNumChildren();
}
