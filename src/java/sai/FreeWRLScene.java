package sai;
import org.web3d.x3d.sai.*;
import java.util.*;


public class FreeWRLScene implements X3DScene {
	ArrayList rNodes;
	ArrayList routes;
	FWComponentInfo[] components;
	FWProfileInfo profile;
	FreeWRLBrowser browser;
	int	encoding;
	String worldURL;
	String specVersion;
	boolean disposed;
	boolean scripted;
	boolean current;
	HashMap metaData;

	public FreeWRLScene(FreeWRLNode[] n, FreeWRLBrowser b) {
		rNodes = new ArrayList(1);
		routes = new ArrayList(1);
		metaData = new HashMap();
		disposed = false;
		
		browser = b;	
		components = null;
		profile = FWProfInfo.getProfile(new String("Full"));
		if (n != null) {
			scripted = true;
			current = false;
			for (int i = 0; i < n.length; i++) {
				rNodes.add(n[i]);
			}
		} else {
                	FreeWRLNode root;
                	X3DNode[] nodes;
			int i;
			int numRoutes;
			FWRoute route;
			FreeWRLNode tonode, fromnode;
			String tofield, fromfield;
			StringTokenizer tokens;
			String retval;
			int numnodes;
			scripted = false;
			current = true;
                	root = (FreeWRLNode) browser.getNode("_Sarah_this_is_the_FreeWRL_System_Root_Node");
                	//root = (FreeWRLNode) browser.getNode("ROOTNODE");
                	MFNode children = (MFNode) root.getField("children");
			numnodes = children.size();
			nodes = new X3DNode[numnodes];
			children.getValue(nodes);
			for (i = 0; i < numnodes; i++) {
				rNodes.add(nodes[i]);
			}
			retval = browser.sendGlobalCommand("j");
                        
                	tokens = new StringTokenizer(retval);
                        
                	numRoutes = Integer.parseInt(tokens.nextToken());
                        
                	for (i = 0; i < numRoutes; i++) {
                        	fromnode = new FreeWRLNode(browser);
                        	fromnode.setPerlPtr(tokens.nextToken());
                        	fromnode.setPointer(tokens.nextToken());
                        	fromfield = tokens.nextToken();
                        	tonode = new FreeWRLNode(browser);
                        	tonode.setPerlPtr(tokens.nextToken());
                        	tonode.setPointer(tokens.nextToken());
                        	tofield = tokens.nextToken();
                        	route = new FWRoute(fromnode, fromfield, tonode, tofield);
				routes.add(route);
                	} 
		} 
	}

	public FreeWRLScene(FreeWRLBrowser b) {
		disposed = false;
		metaData = new HashMap();
		rNodes = new ArrayList(1);
		routes = new ArrayList(1);
		browser = b;
		components = null;
		scripted = true;
		current = false;
		profile = FWProfInfo.getProfile(new String("Full"));
	}

	public FreeWRLScene(FWComponentInfo[] c, FWProfileInfo p, FreeWRLBrowser b) {
		disposed = false;
		metaData = new HashMap();
		scripted = true;
		current = false;
		browser = b;
		components = c;
		profile = p; 
		int i, j;
		boolean flag;
		ArrayList tempcomps;
		rNodes = new ArrayList(1);
		routes = new ArrayList(1);

		if ((profile != null) && (components != null)) {
			ComponentInfo[] profcomps = profile.getComponents();
			tempcomps = new ArrayList(1);

			for (i = 0; i < profcomps.length; i++) {
				flag = false;
				for (j = 0; j < components.length; j++) {
					if ((profcomps[i].getName()).equals(components[j].getName())) {
						if ((profcomps[i].getLevel()) > (components[j].getLevel())) {
							tempcomps.add(profcomps[i]);
							flag = true;
						} else {
							tempcomps.add(components[j]);
							flag = true;
						}
					}
				}
				if (!flag) {
					tempcomps.add(profcomps[i]);
				}
			}

			for (j = 0; j < components.length; j++) {
				flag = false;
				for (i = 0; i < profcomps.length; i++) {
					if ((profcomps[i].getName()).equals(components[j].getName())) {
						flag = true;
					}
				}
				if (!flag) {
					tempcomps.add(components[j]);
				}
			}
				
			Object[] returnval = tempcomps.toArray();

			components = new FWComponentInfo[returnval.length];
		
			for (i = 0; i < returnval.length; i++) {
				components[i] = (FWComponentInfo) returnval[i];
			}
		}

	}

	public void setCurrent(boolean val) {
		current = val;
	}

	public String getMetaData(String key) throws InvalidExecutionContextException {
		return (String) metaData.get(key);
	}

	public void setMetaData(String key, String value) throws InvalidExecutionContextException {
		metaData.put(key, value);
	}

	public X3DNode getExportedNode(String nodeName) throws InvalidExecutionContextException, NodeUnavailableException, InvalidNameException {
		checkValid();
		return browser.getNode(nodeName);
	}

	public void updateExportedNode(String nodeName, String newName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;

		X3DNode nodeRef;

		checkValid();

		nodeRef = browser.getNode(nodeName);

		retval = browser.sendGlobalCommand("c " + newName + " " + nodeRef);

		if (retval.equals("1")) {
			throw new InvalidNameException("Unable to update node name: " + nodeName);
		}
	}

	public void removeExportedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;
		
		checkValid();

		retval = browser.sendGlobalCommand("d " + nodeName);

		if (retval.equals("1")) {
			throw new InvalidNameException("Unable to remove node name: " + nodeName);
		}
	}

	public void addRootNode(X3DNode rootNode) throws InvalidExecutionContextException, NodeInUseException, InsufficientCapabilitiesException {
		FreeWRLNode root;
		X3DNode[] nodes;

		checkValid();

		if (rootNode == null) {
			return;
		}

		nodes = new X3DNode[1];
		nodes[0] = rootNode;
		rNodes.add(rootNode);
		if (current) {
			root = (FreeWRLNode) browser.getNode("_Sarah_this_is_the_FreeWRL_System_Root_Node");
			MFNode addChildren = (MFNode) root.getField("addChildren");
			addChildren.setValue(1, nodes);
		}
	}

	public void removeRootNode(X3DNode rootNode) throws InvalidExecutionContextException {
                FreeWRLNode root;
		int index;
                X3DNode[] nodes;

		checkValid();

                if (rootNode == null) {
                        return;
                } 

                nodes = new X3DNode[1];
                nodes[0] = rootNode;
		index = rNodes.indexOf(rootNode);	
		if (index == -1) {
			System.out.println("Node not present");
			return;
		}
                rNodes.remove(index);
		if (current) {
                	root = (FreeWRLNode) browser.getNode("_Sarah_this_is_the_FreeWRL_System_Root_Node");
                	MFNode removeChildren = (MFNode) root.getField("removeChildren");
                	removeChildren.setValue(1, nodes);
		}
	} 

	public String getSpecificationVersion() throws InvalidExecutionContextException {
		String spec;
		checkValid();
		spec = browser.sendGlobalCommand("Y");
		int version = Integer.parseInt(spec);
		if (version == 3) {
			spec = new String("2.0");
		} else if (version == 4) {
			spec = new String("1.0");
		} else {
			spec = new String("0.0");
		}
		return spec;
	}

	public int getEncoding() throws InvalidExecutionContextException {
		String spec;
		checkValid();
		spec = browser.sendGlobalCommand("Y");
		return Integer.parseInt(spec);
	}

	public ProfileInfo getProfile() throws InvalidExecutionContextException {
		checkValid();
		return profile;
	}

        public ComponentInfo[] getComponents() throws InvalidExecutionContextException {
		checkValid();
		return components;
	}

        public String getWorldURL() throws InvalidExecutionContextException {
		String myurl = null;
		checkValid();
		if (!scripted)
			myurl = browser.sendGlobalCommand("O");
		return myurl;
			
	}

        public X3DNode getNamedNode(String nodeName) throws InvalidExecutionContextException, NodeUnavailableException, InvalidNameException {
		checkValid();
		return browser.getNode(nodeName);
	}

        public X3DNode getImportedNode(String nodeName) throws InvalidExecutionContextException,  NodeUnavailableException, InvalidNameException {
		checkValid();
		return browser.getNode(nodeName);
	}

        public X3DNode createNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException {
		String retval;
		StringTokenizer tokens;
		FreeWRLNode newnode;

		checkValid();

                if (nodeName == null) {
                        throw new InvalidNameException("No node of type " + nodeName + " exists");
                }

		retval = browser.sendGlobalCommand("a " + nodeName);

		if (retval.equals("")) {
			throw new InvalidNameException("No node of type " + nodeName + " exists");
		}
		tokens = new StringTokenizer(retval);
		newnode = new FreeWRLNode(browser);
		newnode.setPerlPtr(tokens.nextToken());
		newnode.setPointer(tokens.nextToken());
		newnode.setType(FreeWRLFieldTypes.getIntType("h"));

		return newnode;
	}

        public X3DProtoInstance createProto(String protoName) throws InvalidExecutionContextException, InvalidNameException {
		String retval;
		StringTokenizer tokens;
		FWProtoInstance proto;

		checkValid();

                if (protoName == null) {
                        throw new InvalidNameException("No PROTO of type " + protoName + " exists");
                }

		retval = browser.sendGlobalCommand("b " + protoName);
                if (retval.equals("")) {
                        throw new InvalidNameException("No PROTO of type " + protoName + " exists");
                }
		tokens = new StringTokenizer(retval);
		proto = new FWProtoInstance(browser);
		proto.setPerlPtr(tokens.nextToken());
		proto.setPointer(tokens.nextToken());
		proto.setNodeName(protoName);
		proto.setType(FreeWRLNodeTypes.X3DProtoInstance);

		return proto;

	}

        public void updateNamedNode(String nodeName, X3DNode nodeRef) throws InvalidExecutionContextException, InvalidNameException, ImportedNodeException {
                String retval;

		checkValid();

		retval = browser.sendGlobalCommand("c " + nodeName + " " + nodeRef);

		if (retval.equals("1")) {
			throw new InvalidNameException("Unable to update node name: " + nodeName);
		}
	}

        public void updateImportedNode(String nodeName, String importedName, X3DNode nodeRef) throws InvalidExecutionContextException, InvalidNameException, ImportedNodeException {
		checkValid();
	}

        public void removeNamedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;
		
		checkValid();

		retval = browser.sendGlobalCommand("d " + nodeName);

		if (retval.equals("1")) {
			throw new InvalidNameException("Unable to remove node name: " + nodeName);
		}
	}

        public void removeImportedNode(String nodeName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;

		checkValid();

		retval = browser.sendGlobalCommand("d " + nodeName);

                if (retval.equals("1")) {
                        throw new InvalidNameException("Unable to remove imported node name: " + nodeName);
                }
	}

        public X3DProtoDeclaration getProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;
		StringTokenizer tokens;
		FWProtoDeclaration dec;
		FreeWRLFieldDefinition[] defs;
		int numFields;
		String type;
		String fieldType;
		String fieldAccess;
		String fieldName;
		String temp;
		String defaultValue;
		int ft, fa;

		checkValid();
                retval = browser.sendGlobalCommand("e " + protoName);

                if (retval.equals("1")) {
                        throw new InvalidNameException("The proto declaration " + protoName + " does not exist");
                }


		dec = new FWProtoDeclaration(browser);
		tokens = new StringTokenizer(retval);

		dec.setProtoName(tokens.nextToken());
		type = tokens.nextToken();
		numFields = Integer.valueOf(tokens.nextToken()).intValue();

		defs = new FreeWRLFieldDefinition[numFields];

		for (int i = 0; i < numFields; i++) {
			fieldAccess = tokens.nextToken();
			fieldType = tokens.nextToken();
			fieldName = tokens.nextToken();
			ft = FreeWRLFieldTypes.getIntFromStringDesc(fieldType);
			fa = FreeWRLFieldTypes.getAccessFromType(fieldAccess);
			defs[i] = new FreeWRLFieldDefinition(fieldName, fa, ft);
			defaultValue = tokens.nextToken();

			temp = "";

			char test = '\"';
			String tests = "\"";
			if (defaultValue.equals("["))  {
				while(!temp.equals("]")) {
					temp = tokens.nextToken();
					defaultValue = defaultValue + " " + temp;
				}
			}	

			if (defaultValue.charAt(0) == test) {
				while(!temp.endsWith(tests)) {
					temp = tokens.nextToken();
					defaultValue = defaultValue + " " + temp;
				}
			}

			defs[i].setDefaultValue(defaultValue);
		}

		dec.setFields(defs);

		return dec;
	}

        public void updateProtoDeclaration(String protoName, X3DProtoDeclaration newDeclaration) throws InvalidExecutionContextException, InvalidNameException {
                String retval;
		checkValid();

		retval = browser.sendGlobalCommand("f " + protoName + " " + newDeclaration);
		System.out.println("sent: f " + protoName + " " + newDeclaration);

                if (retval.equals("1")) {
                        throw new InvalidNameException("Proto name " + protoName + " is already in use");
                }

	}

        public void removeProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException {
                String retval;

		checkValid();
                if (protoName == null) {
                        throw new InvalidNameException("The proto declaration " + protoName + " does not exist");
                }
                retval = browser.sendGlobalCommand("g " + protoName);

                if (retval.equals("1")) {
                        throw new InvalidNameException("Unable to remove proto name: " + protoName);
                }
	}

        public X3DExternProtoDeclaration getExternProtoDeclaration(String protoName) throws InvalidExecutionContextException, InvalidNameException, URLUnavailableException {
		return (X3DExternProtoDeclaration) getProtoDeclaration(protoName);
	}

        public void updateExternProtoDeclaration(String protoName, X3DExternProtoDeclaration newDeclaration) throws InvalidExecutionContextException {
		updateProtoDeclaration(protoName, newDeclaration);
	}

        public void removeExternProtoDeclaration(String protoName) throws InvalidExecutionContextException {
		removeProtoDeclaration(protoName);
	}

        public X3DNode[] getRootNodes() throws InvalidExecutionContextException {
		checkValid();
		Object[] returnval = rNodes.toArray();
		FreeWRLNode[] temp = new FreeWRLNode[rNodes.size()];
		for (int i = 0; i < rNodes.size(); i++) 
			temp[i] = (FreeWRLNode) returnval[i];
		return temp;
	}

	public X3DRoute[] getRoutes() throws InvalidExecutionContextException {
		checkValid();
		Object[] returnval = routes.toArray();
		FWRoute[] temp = new FWRoute[routes.size()];
		for (int i = 0; i < routes.size(); i++) 
			temp[i] = (FWRoute) returnval[i];
		return temp;
	}

        public X3DRoute addRoute(X3DNode startNode, String startName, X3DNode endNode, String endEvent)  throws InvalidExecutionContextException, InvalidNodeException, InvalidFieldException {
		checkValid();
		FWRoute route;
		String retval;
		retval = browser.addRoute((FreeWRLNode) startNode, startName, (FreeWRLNode) endNode, endEvent);
		System.out.println("retval: " + retval);
		if (retval.equals("1")) {
			throw new InvalidFieldException("Unable to create ROUTE");
		}
		route = new FWRoute((FreeWRLNode) startNode, startName, (FreeWRLNode) endNode, endEvent);
		routes.add(route);
		return route;
	}

        public void removeRoute(X3DRoute route)  throws InvalidExecutionContextException, InvalidNodeException, InvalidFieldException {
		checkValid();
		int index;
		if (route == null) {
			throw new InvalidNodeException("No such route exists");
		}

		index = routes.indexOf(route);
		if (index == -1) {
			System.out.println("Route not present");
			return;
		}

		routes.remove(index);

		browser.deleteRoute((FreeWRLNode)(route.getSourceNode()), route.getSourceField(), (FreeWRLNode)(route.getDestinationNode()), route.getDestinationField());
	}

	public void checkValid() {
		if (disposed) {
			throw new InvalidExecutionContextException("This context has been disposed");
		}
	}

	public void dispose() {
		disposed = true;
	}
}
