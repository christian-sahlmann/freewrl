package org.web3d.x3d.sai;
import java.awt.image.*;

public interface MFImage extends MField {

	public int getWidth(int imgIndex);
	public int getHeight(int imgIndex);
	public int getComponents(int imgIndex);
	public void getPixels(int imgIndex, int[] pixels);
	public WritableRenderedImage getImage(int imgIndex);
	public void setImage(int imgIndex, RenderedImage img);
	public void setSubImage(int imgIndex, RenderedImage img, int srcWidth, int srcHeight, int srcXOffset, int srcYOffset, int destXOffset, int destYOffset);
	public void set1Value(int index, int value);
	public void set1Value(int imgIndex, int width, int height, int components, int[] pixels);
	public void setValue(int[] value);
	public void setImage(RenderedImage[] img);
	public void append(RenderedImage value);
	public void insertValue(int index, RenderedImage value);
}
