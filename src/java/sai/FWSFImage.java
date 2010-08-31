package sai;
import org.web3d.x3d.sai.*;

import java.awt.*;
import java.awt.image.*;

public class FWSFImage extends FreeWRLField implements SFImage {
	FreeWRLBrowser browser;
	
	public FWSFImage(FreeWRLFieldDefinition def, FreeWRLBrowser b) {
		super(def, b);
		browser = b;
	}

	public int getWidth() {
		return -1;
	}

	public int getHeight() {
		return -1;
	}

	public int getComponents() {
		return -1;
	}

	public void getPixels(int[] pixels) {
	}

	public WritableRenderedImage getImage() {
		return null;
	}

	public void setValue(int width, int height, int components, int[] pixels) {
	}
	
	public void setImage(RenderedImage image) {
	}
	
	public void setSubImage(RenderedImage image, int srcWidth, int srcHeight, int srcXOffset, int srcYoffset, int destXOffset, int destYOffset) {
	}
}
