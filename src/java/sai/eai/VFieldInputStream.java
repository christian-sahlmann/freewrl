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

import java.io.*;

class VFieldInputStream extends DataInputStream {

    VFieldInputStream(InputStream in) {
	super(in);
    }

    VField readField() throws IOException {
	byte type = readByte();
	switch(type) {
	  case VField.NOTHING:
	    return null;
	  case VField.SFBOOL:
	    return new VSFBool(this);
	  case VField.SFVEC3F:
	    return new VSFVec3f(this);
	  case VField.SFROTATION:
	    return new VSFRotation(this);
	  case VField.SFSTRING:
	    return new VSFString(this);
	  case VField.MFSTRING:
	    return new VMFString(this);

/*
	  case VField.SFCOLOR:
	    return new VSFColor(this);
	  case VField.SFFLOAT:
	    return new VSFFloat(this);
	  case VField.SFIMAGE:
	    return new VSFImage(this);
	  case VField.SFINT32:
	    return new VSFInt32(this);
	  case VField.SFTIME:
	    return new VSFTime(this);
	  case VField.SFVEC2F:
	    return new VSFVec2f(this);

	  case VField.MFCOLOR:
	    return new VMFColor(this);
	  case VField.MFFLOAT:
	    return new VMFFloat(this);
	  case VField.MFINT32:
	    return new VMFInt32(this);
	  case VField.MFROTATION:
	    return new VMFRotation(this);
	  case VField.MFVEC2F:
	    return new VMFVec2f(this);
	  case VField.MFVEC3F:
	    return new VMFVec3f(this);
*/

	  default:
	    throw new UnsupportedFieldTypeException("type " + type);
	}
    }
}

