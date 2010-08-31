package vrml;
import java.io.*;
import java.net.URL;
import java.net.SocketPermission;
import java.net.MalformedURLException;
import java.util.Enumeration;
import java.util.NoSuchElementException;
import java.util.PropertyPermission;
import java.security.*;
import java.security.cert.Certificate;


public final class FWJavaScriptClassLoader extends SecureClassLoader {
    URL baseURL;
    CodeSource myCodeSource;


    /**
     * @param url base url for loading classes.
     */
    public FWJavaScriptClassLoader(String url) {
	System.out.println ("new FWJavaScriptClassLoader - url is " + url);
	try {
	    baseURL = new java.net.URL(url);

	    myCodeSource = new CodeSource(baseURL, (Certificate[]) null);
	} catch (MalformedURLException ex) {
	    throw new InternalError("Script URL malformed: "+url);
	}
    }

    protected Class findClass(String name)
	throws ClassNotFoundException
    {
	System.err.println("LOADING CLASS '"+name+"'");
	try {
	    byte[] b = readFile(name.replace('.', '/') + ".class");
	    return defineClass(name, b, 0, b.length, myCodeSource);
	} catch (IOException e) {
	    throw new ClassNotFoundException(name);
	}
    }

    private static final String props[] = {
	"file.separator",
	"path.separator",
	"java.class.version",
	"java.vendor",
	"java.version",
	"java.specification.name",
	"java.specification.vendor",
	"java.specification.version",
	"java.vendor.url",
	"java.vm.name",
	"java.vm.vendor",
	"java.vm.version",
	"java.vm.specification.name",
	"java.vm.specification.vendor",
	"java.vm.specification.version",
	"line.separator",
	"os.name",
	"os.arch",
	"os.version"
    };

    protected PermissionCollection getPermissions(CodeSource codesource) {
	Permissions perms = new Permissions();
	URL url = codesource.getLocation();
	perms.add(new SocketPermission(url.getHost(), "connect,accept"));
	if (url.getProtocol().equals("file")) {
	    /* local script */
	    String path = url.getFile().replace('/', File.separatorChar);
	    path = path.substring(0, path.lastIndexOf(File.separatorChar)+1);
	    perms.add(new FilePermission(path+"*", "read,write,delete"));
	}
	for (int i = 0; i < props.length; i++)
	    perms.add(new PropertyPermission(props[i], "read"));
	//System.err.println("Script permission are "+perms);
	return perms;
    }

    private byte[] readFile(String name) throws IOException
    {
	InputStream is = getResourceAsStream(name);
	ByteArrayOutputStream bao = new ByteArrayOutputStream();
	byte[] buff = new byte[4096];
	int len;
	while ((len = is.read(buff)) > 0)
	    bao.write(buff, 0, len);
	return bao.toByteArray();
    }

    protected URL findResource(String name)
    {
	try {
	    System.err.println("LOADING RESOURCE '"+name+"'");
	    return new URL(baseURL, name);
	} catch (MalformedURLException ex) {
	    return null;
	}
    }

    protected Enumeration findResources(String name) throws IOException
    {
	final URL url = new URL(baseURL, name);
	return new Enumeration() {
		boolean hasMore = true;
		public boolean hasMoreElements() {
		    return hasMore;
		}
		public Object nextElement() {
		    if (!hasMore)
			throw new NoSuchElementException();
		    hasMore = false;
		    return url;
		}
	    };
    }
}
