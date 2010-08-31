package org.web3d.x3d.sai;

public interface X3DComponent {

	public ExternalBrowser getBrowser();
	public Object getImplementation();
	public void shutdown();
}
