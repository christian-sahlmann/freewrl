package org.web3d.x3d.sai;

public interface X3DScriptImplementation {
	public void setBrowser(Browser browser);
	public void setFields(X3DScriptNode externalView, java.util.Map fields);
	public void initialize();
	public void eventsProcessed();
	public void shutdown();
}
