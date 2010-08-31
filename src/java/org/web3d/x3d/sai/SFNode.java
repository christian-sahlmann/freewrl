package org.web3d.x3d.sai;

public interface SFNode extends X3DField {
	public X3DNode getValue();
	public void setValue(X3DNode value) throws InvalidNodeException;
}
