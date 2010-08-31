package vrml.external;

import vrml.external.Node;
import vrml.external.exception.InvalidVrmlException;

public interface IBrowser {
	public String getName();
	public String getVersion();
	public int getEncoding();
	public float getCurrentSpeed();
	public float getCurrentFrameRate();
	public String getWorldURL();
	public void replaceWorld(Node[] nodes) throws IllegalArgumentException;
	public void loadURL(String[] url, String[] parameter);
	public void setDescription(String description);
	public String getDescription();
	public String getRenderingProperties();
	public Node[] createVrmlFromString(String vrmlSyntax) throws InvalidVrmlException;
	public void createVrmlFromURL(String[] url, Node node, String event);
	public Node getNode(String name);
	public void addRoute(Node fromNode, String fromEventOut, Node toNode, String toEventIn) throws IllegalArgumentException;
	public void deleteRoute(Node fromNode, String fromEventOut, Node toNode, String toEventIn) throws IllegalArgumentException;
	public void beginUpdate();
	public void endUpdate();
	public void initialize();
	public void shutdown();
	public void firstViewpoint();
	public void lastViewpoint();
	public void nextViewpoint();
	public void previousViewpoint();
	public String createNode (String name);
	public String createProto (String name);
	public String updateNamedNode (String name, Node node);
	public String removeNamedNode (String name);
	public String getProtoDeclaration (String name);
	public String removeProtoDeclaration (String name);
	public String updateProtoDeclaration (String name, String npdecl);
	public String getNodeFieldDefs (Node myn);
	public String getNodeDEFName(Node myn);
}
