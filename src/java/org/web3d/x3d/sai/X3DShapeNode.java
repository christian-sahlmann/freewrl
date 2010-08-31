package org.web3d.x3d.sai;

public interface X3DShapeNode extends X3DChildNode {
	public X3DNode getAppearance();
	public void setAppearance(X3DAppearanceNode app);
	public void setAppearance(X3DProtoInstance app);
	public X3DNode getGeometry();
	public void setGeometry(X3DGeometryNode geom);
	public void setGeometry(X3DProtoInstance geom);
}
