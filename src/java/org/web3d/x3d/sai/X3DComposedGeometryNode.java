package org.web3d.x3d.sai;

public interface X3DComposedGeometryNode extends X3DGeometryNode {
	public X3DNode getColor();
	public void setColor(X3DColorNode node);
	public void setColor(X3DProtoInstance comp);
	public X3DNode getCoord();
	public void setCoord(X3DCoordinateNode node);
	public void setCoord(X3DProtoInstance node);
	public X3DNode getTexCoord();
	public void setTexCoord(X3DTextureCoordinateNode node);
	public void setTexCoord(X3DProtoInstance node);
	public X3DNode getNormal();
	public void setNormal(X3DNormalNode node);
	public void setNormal(X3DProtoInstance node);
	public boolean getIsSolid();
	public void setIsSolid(boolean solid);
	public boolean getIsCCW();
	public void setIsCCW(boolean ccw);
	public boolean getColorPerVertex();
	public void setColorPerVertex(boolean perVertex);
	public boolean getNormalPerVertex();
	public void setNormalPerVertex(boolean perVertex);
}
