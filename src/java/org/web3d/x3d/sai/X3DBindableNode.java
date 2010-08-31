package org.web3d.x3d.sai;

public interface X3DBindableNode extends X3DChildNode {
	public void setBind(boolean enable);
	public boolean isBound();
	public double getBindTime();
}
