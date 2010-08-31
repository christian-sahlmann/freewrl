package vrml;
import java.io.*;

public abstract class Field implements Cloneable
{
    FWJavaScriptBinding __binding = null;
    String offset = "0 -1 0"; // type, offset of -1, length of zero

    public Object clone() {
	try {
	    Field f = (Field) super.clone();
	    f.__binding = null;
	    return f;
	} catch (CloneNotSupportedException ex) {
	    throw new InternalError();
	}
    }

    public void bind_to(FWJavaScriptBinding b) {
	__binding = b;
    }

    public final void __updateRead() {
	if (__binding != null)
	    __binding.updateRead(this);
    }
    protected final void __updateWrite() {
	if (__binding != null)
	    __binding.updateWrite(this);
    }

    public abstract void __fromPerl(BufferedReader in) throws IOException;
    public abstract void __toPerl(PrintWriter out) throws IOException;

    public void setOffset(String offs) { this.offset = offs; } //JAS2
    public String getOffset() { return this.offset; } //JAS2


}


