package org.web3d.x3d.sai;

public interface SFImage extends X3DField {
	public int getWidth();
	public int getHeight();
	public int getComponents();
	public void getPixels(int[] pixels);
	public java.awt.image.WritableRenderedImage getImage();
	public void setValue(int width, int height, int components, int[] pixels);
	public void setImage(java.awt.image.RenderedImage image);
	public void setSubImage(java.awt.image.RenderedImage image, int srcWidth, int srcHeight, int srcXOffset, int srcYOffset, int destXOffset, int destYOffset);
}
