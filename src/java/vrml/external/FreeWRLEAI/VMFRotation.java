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

package vrml.external.FreeWRLEAI;

import java.io.*;

public class VMFRotation extends VField
{
    VSFRotation[]	values;

    public VMFRotation(DataInputStream in) throws IOException
    {
	values = new VSFRotation[in.readInt()];
	for (int i = 0; i < values.length; i++) {
	    values[i] = new VSFRotation(in);
	}
    }

    public void write(DataOutputStream out) throws IOException
    {
	out.writeInt(values.length);
	for (int i = 0; i < values.length; i++) {
	    values[i].write(out);
	}
    }

    public byte getType() { return MFROTATION; }
}
