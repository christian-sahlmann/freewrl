package org.web3d.x3d.sai;

import java.applet.Applet;
import java.net.*;
import java.util.*;
import java.awt.*;

public interface BrowserFactoryImpl {

	public ExternalBrowser getBrowser(Applet applet) throws NotSupportedException, NoSuchBrowserException, ConnectionException;

	public ExternalBrowser getBrowser(Applet applet, String frameName, int index) throws NotSupportedException, NoSuchBrowserException, ConnectionException;

	public ExternalBrowser getBrowser(InetAddress add, int port) throws NotSupportedException, NoSuchBrowserException, UnknownHostException, ConnectionException;
	public X3DComponent createX3DComponent(Map args) throws NotSupportedException;	
	
}
