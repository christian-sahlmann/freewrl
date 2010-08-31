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

public class VSFVec2f extends VField
{
    float	x, y;

    public VSFVec2f(float x, float y, float z)
    {
	this.x = x;
	this.y = y;
    }

    public VSFVec2f(DataInputStream in) throws IOException
    {
	x = in.readFloat();
	y = in.readFloat();
    }

    public void write(DataOutputStream out) throws IOException
    {
	out.writeFloat(x);
	out.writeFloat(y);
    }

    public byte getType() { return SFVEC2F; }
}
