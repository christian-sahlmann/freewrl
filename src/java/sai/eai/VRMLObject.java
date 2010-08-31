// copyright (c) 1997,1998 stephen f. white
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

package sai.eai;

//JAS import vrml.external.*;
//JAS import vrml.external.field.*;
//JAS import java.util.Hashtable;

public class VRMLObject
{
    public int				id;
    public String			URL;
    public VRMLObject			next;
    public String[]			gestures;
    public boolean			loaded = false;

    protected String			name;
    protected String []			fieldNames;
    protected VRMLObjectObserver	observer;
    protected VField []			fields;

    public VRMLObject(int id, String URL, VRMLObjectObserver observer)
    {
	this.id = id;
	this.URL = URL;
	this.observer = observer;
	fieldNames = new String[VIP.NUM_FIELDS];
	for (short i = 0; i < VIP.NUM_FIELDS; i++) {
	    fieldNames[i] = VIP.fieldName(i);
	}
	fields = new VField[4];
	fields[VIP.POSITION] = new VSFVec3f(0.0F, 0.0F, 10.0F);
	fields[VIP.ORIENTATION] = new VSFRotation(0.0F, 0.0F, 1.0F, 0.0F);
	fields[VIP.SCALE] = new VSFVec3f(1.0F, 1.0F, 1.0F);
	String[]	strs = { "" };
	fields[VIP.NAME] = new VMFString(strs);
    }

    public String[]  getFieldNames() {
	return fieldNames;
    }

    public VField  getField(short field) {
	return fields[field];
    }

    public void  setName(String name) {
	this.name = name;
    }

    public void  setField(short field, VField value) {
	try {
	    fields[field] = value;
	    doSetField(field, value);
	    if (field == VIP.NAME) {
		setName(((VMFString) value).get1Value(0));
	    }
	} catch (ArrayIndexOutOfBoundsException e) {
	    System.err.println("unknown field " + field + " in " + this);
	}
    }

    public String toString()
    {
	return (name != null ? name : "" ) + "(" + id + ")";
    }

    protected void  doSetField(short field, VField value) {}
    public void  load() {}
}
