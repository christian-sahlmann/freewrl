package org.web3d.x3d.sai;

public interface X3DRoute {
	public X3DNode getSourceNode() throws InvalidOperationTimingException, InvalidRouteException;
	public String getSourceField() throws InvalidOperationTimingException, InvalidRouteException;
	public X3DNode getDestinationNode() throws InvalidOperationTimingException, InvalidRouteException;
	public String getDestinationField() throws InvalidOperationTimingException, InvalidRouteException;
	public void dispose() throws InvalidOperationTimingException;
}
