package vrml;
import java.util.Vector;

public abstract class ConstMField extends ConstField
{
    public Vector __vect = new Vector();

    public int getSize() {
	__updateRead();
	return __vect.size();
    }

    protected final void __update1Read(int index) {
	__updateRead();
    }
}


