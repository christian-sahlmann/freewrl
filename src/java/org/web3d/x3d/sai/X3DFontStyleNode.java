package org.web3d.x3d.sai;
import java.awt.Font;

public interface X3DFontStyleNode extends X3DNode {
	public int PLAIN_STYLE = java.awt.Font.PLAIN;
	public int ITALIC_STYLE = java.awt.Font.ITALIC;
	public int BOLD_STYLE = java.awt.Font.BOLD;
	public int BOLDITALIC_STYLE = java.awt.Font.BOLD + java.awt.Font.ITALIC;
	public int BEGIN_JUSTIFY = 1;
	public int END_JUSTIFY = 2;
	public int MIDDLE_JUSTIFY = 3;
	public int FIRST_JUSTIFY = 4;

	public Font getFont();
	public int getHorizontalJustification();
	public int getVerticalJustification();
	public float getSpacing();
	public float getSize();
	public boolean isTopToBottom();
	public boolean isLeftToRight();
}
