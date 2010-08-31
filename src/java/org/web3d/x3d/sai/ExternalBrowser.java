package org.web3d.x3d.sai;
import vrml.external.*;

public interface ExternalBrowser extends Browser{
	public void addBrowserListener(BrowserListener listener) throws InvalidBrowserException;
	public void removeBrowserListener(BrowserListener l) throws InvalidBrowserException;
	public void beginUpdate() throws InvalidBrowserException;
	public void endUpdate() throws InvalidBrowserException;
	public void dispose() throws InvalidOperationTimingException;
}
