package org.web3d.x3d.sai;
import org.w3c.dom.Node;
import java.util.*;

public interface Browser {
	public X3DScene importDocument(Node element) throws InvalidBrowserException, InvalidDocumentException, NotSupportedException, ConnectionException;
	public String getName() throws InvalidBrowserException, ConnectionException;
	public String getVersion() throws InvalidBrowserException, ConnectionException;
	public ProfileInfo getProfile(String name) throws InvalidBrowserException, NotSupportedException, ConnectionException;
	public ProfileInfo[] getSupportedProfiles() throws InvalidBrowserException, ConnectionException;
	public ComponentInfo[] getSupportedComponents() throws InvalidBrowserException, ConnectionException;
	public ComponentInfo getComponent(String name, int level) throws InvalidBrowserException, NotSupportedException, ConnectionException;
	public X3DExecutionContext getExecutionContext() throws InvalidBrowserException, ConnectionException;
	public X3DScene createScene(ProfileInfo profile, ComponentInfo[] components) throws InvalidBrowserException, ConnectionException;
	public float getCurrentSpeed() throws InvalidBrowserException, ConnectionException;
	public float getCurrentFrameRate() throws InvalidBrowserException, ConnectionException;	
	public void replaceWorld(X3DScene scene) throws InvalidBrowserException, ConnectionException;
	public void loadURL(String[] url, Map parameters) throws InvalidBrowserException, InvalidURLException, ConnectionException;
	public String getDescription() throws InvalidBrowserException, ConnectionException;
	public void setDescription (String desc) throws InvalidBrowserException, ConnectionException;
	public X3DScene createX3DFromString(String scene) throws InvalidBrowserException, InvalidX3DException, NotSupportedException, ConnectionException;
	public X3DScene createX3DFromStream(java.io.InputStream is) throws InvalidBrowserException, InvalidX3DException, NotSupportedException, java.io.IOException, ConnectionException;
	public X3DScene createX3DFromURL(String[] url) throws InvalidBrowserException, InvalidX3DException, ConnectionException, java.io.IOException;
	public java.util.Map getRenderingProperties() throws InvalidBrowserException, ConnectionException;
	public java.util.Map getBrowserProperties() throws InvalidBrowserException, ConnectionException;
	public void nextViewpoint() throws InvalidBrowserException, ConnectionException;
	public void previousViewpoint() throws InvalidBrowserException, ConnectionException;
	public void firstViewpoint() throws InvalidBrowserException, ConnectionException;
	public void lastViewpoint() throws InvalidBrowserException, ConnectionException;
	public void print(Object obj) throws InvalidBrowserException, ConnectionException;
	public void println(Object obj) throws InvalidBrowserException, ConnectionException;
	public void dispose();
}
