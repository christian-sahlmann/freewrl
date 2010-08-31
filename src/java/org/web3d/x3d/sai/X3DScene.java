package org.web3d.x3d.sai;

public interface X3DScene extends X3DExecutionContext {
	public String getMetaData(String key) throws InvalidExecutionContextException;
	public void setMetaData(String key, String value) throws InvalidExecutionContextException;
	public X3DNode getExportedNode(String nodeName) throws InvalidExecutionContextException, NodeUnavailableException, InvalidNameException;
	public void updateExportedNode(String nodeName, String newName) throws InvalidExecutionContextException, InvalidNameException;
	public void removeExportedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException;
	public void addRootNode(X3DNode rootNode) throws InvalidExecutionContextException, NodeInUseException, InsufficientCapabilitiesException;
	public void removeRootNode(X3DNode rootNode) throws InvalidExecutionContextException;
	public void dispose();
}
