package org.web3d.x3d.sai;

public interface X3DTextNode extends X3DGeometryNode {
	public void setFontStyle(X3DFontStyleNode fs);
	public void setFontStyle(X3DProtoInstance fs);
	public X3DNode getFontStyle();
	public int getNumText();
	public void setText(String[] text);
	public void getText(String[] text);
}
