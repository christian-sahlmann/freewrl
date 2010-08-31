package vrml;
import java.util.Vector;

public abstract class MField extends Field
{
    public Vector __vect = new Vector();

    public int getSize() {
	__updateRead();
	return __vect.size();
    }

    public void clear() {
	__vect.clear();
	__updateWrite();
    }

    public void delete(int index) {
	__updateRead();
	__vect.removeElementAt(index);
	__updateWrite();
    }

    protected final void __update1Read(int index) {
	__updateRead();
    }

    protected final void __set1Value(int index, ConstField fld) {
	__updateRead();
	fld.__updateRead();
	__vect.setElementAt(fld, index);
	__updateWrite();
    }

    protected final void __insertValue(int index, ConstField fld) {
	__updateRead();
	fld.__updateRead();
	__vect.insertElementAt(fld, index);
	__updateWrite();
    }

    protected final void __addValue(ConstField fld) {
	__updateRead();
	fld.__updateRead();
	__vect.addElement(fld);
	__updateWrite();
    }
}


