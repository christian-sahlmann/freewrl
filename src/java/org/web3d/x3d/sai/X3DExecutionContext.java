package org.web3d.x3d.sai;

public interface X3DExecutionContext {
	String getSpecificationVersion() throws InvalidExecutionContextException;
	int getEncoding() throws InvalidExecutionContextException;
	ProfileInfo getProfile() throws InvalidExecutionContextException;
	ComponentInfo[] getComponents() throws InvalidExecutionContextException;
	String getWorldURL() throws InvalidExecutionContextException;
	X3DNode getNamedNode(String nodeName) throws InvalidExecutionContextException, NodeUnavailableException, InvalidNameException;
	X3DNode getImportedNode(String nodeName) throws InvalidExecutionContextException, NodeUnavailableException, InvalidNameException;
	X3DNode createNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException;
	X3DProtoInstance createProto(String protoName) throws InvalidExecutionContextException, InvalidNameException;
	void updateNamedNode(String nodeName, X3DNode nodeRef) throws InvalidExecutionContextException, InvalidNameException, ImportedNodeException;
	void updateImportedNode(String nodeName, String importedName, X3DNode nodeRef) throws InvalidExecutionContextException, InvalidNameException, ImportedNodeException;
	void removeNamedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException;
	void removeImportedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException;
	X3DProtoDeclaration getProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException;
	void updateProtoDeclaration(String protoName, X3DProtoDeclaration newDeclaration) throws InvalidExecutionContextException, InvalidNameException;
	void removeProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException;
	X3DExternProtoDeclaration getExternProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException, URLUnavailableException;
	void updateExternProtoDeclaration(String protoName, X3DExternProtoDeclaration newDeclaration) throws InvalidExecutionContextException;
	void removeExternProtoDeclaration(String protoName) throws InvalidExecutionContextException;
	X3DNode[] getRootNodes() throws InvalidExecutionContextException;
	X3DRoute[] getRoutes() throws InvalidExecutionContextException;
	X3DRoute addRoute(X3DNode startNode, String starttName, X3DNode endNode, String endEvent)  throws InvalidExecutionContextException, InvalidNodeException, InvalidFieldException;
	void removeRoute(X3DRoute route)  throws InvalidExecutionContextException;
}
