package org.web3d.x3d.sai;

public interface X3DExternProtoDeclaration extends X3DProtoDeclaration {
	public int getLoadState() throws InvalidOperationTimingException, InvalidProtoException;
	void loadNow() throws InvalidOperationTimingException, InvalidProtoException;
}
