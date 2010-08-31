package org.web3d.x3d.sai;

public interface X3DProtoDeclaration {
	public X3DProtoInstance createInstance() throws InvalidOperationTimingException, InvalidProtoException;
	public X3DFieldDefinition[] getFieldDefinitions() throws InvalidOperationTimingException, InvalidProtoException;
	public void dispose();
} 
