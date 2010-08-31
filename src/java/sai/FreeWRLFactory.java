package sai;
import org.web3d.x3d.sai.*;

import org.web3d.x3d.sai.*;
import java.applet.Applet;
import java.net.*;
import java.util.*;
import java.awt.*;
import vrml.external.*;
class FreeWRLFactory implements BrowserFactoryImpl {
	public ExternalBrowser getBrowser(Applet applet) throws NotSupportedException, NoSuchBrowserException {
		return new FreeWRLBrowser(applet);	
	}

	public ExternalBrowser getBrowser(Applet applet, String frameName, int index) throws NotSupportedException, NoSuchBrowserException {
		throw new NotSupportedException("getBrowser(Applet, String, int) not supported");	
	}

	public ExternalBrowser getBrowser(InetAddress add, int port) throws NotSupportedException, NoSuchBrowserException, UnknownHostException, ConnectionException {
		throw new NotSupportedException("getBrowser(InetAddress, int) not supported");	
	}

	public X3DComponent createX3DComponent(Map args) throws NotSupportedException {
		throw new NotSupportedException("createX3DComponent not supported");
	}

}
