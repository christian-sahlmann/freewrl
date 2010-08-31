package org.web3d.x3d.sai;

public interface X3DTextureTransform2DNode extends X3DTextureTransformNode {
	public void getCenter(float[] position);
	public void setCenter(float[] position);
	public float getRotation();
	public void setRotation(float angle);
	public void getScale(float[] scale);
	public void setScale(float[] scale);
	public void getTranslation(float[] trans);
	public void setTranslation(float[] trans);
}
