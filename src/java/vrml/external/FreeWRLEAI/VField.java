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

// WARNING:  this is *not* an implementation of the field classes for general
// VRML use.  it's just a bunch of wrappers around the objects for marshalling
// and unmarshalling them from network streams.

public abstract class VField
{
    public static final byte	NOTHING = -1;

    public static final byte	SFBOOL = 0;
    public static final byte	SFCOLOR = 1;
    public static final byte	SFFLOAT = 2;
    public static final byte	SFIMAGE = 3;
    public static final byte	SFINT32 = 4;
    public static final byte	SFNODE = 5;
    public static final byte	SFROTATION = 6;
    public static final byte	SFSTRING = 7;
    public static final byte	SFTIME = 8;
    public static final byte	SFVEC2F = 9;
    public static final byte	SFVEC3F = 10;

    public static final byte	MFCOLOR = 11;
    public static final byte	MFFLOAT = 12;
    public static final byte	MFINT32 = 13;
    public static final byte	MFNODE = 14;
    public static final byte	MFROTATION = 15;
    public static final byte	MFSTRING = 16;
    public static final byte	MFVEC2F = 17;
    public static final byte	MFVEC3F = 18;

    public byte getType() { return NOTHING; }

    public abstract void write(DataOutputStream out) throws IOException;
}
