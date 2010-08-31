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

public class VMFString extends VField
{
    private String[]	strings;

    public VMFString(DataInputStream in) throws IOException
    {
	strings = new String[in.readInt()];
	for (int i = 0; i < strings.length; i++) {
	    strings[i] = in.readUTF();
	}
    }

    public VMFString(String[] strings) {
	this.strings = strings;
    }

    public void write(DataOutputStream out) throws IOException
    {
	out.writeInt(strings.length);
	for (int i = 0; i < strings.length; i++) {
	    out.writeUTF(strings[i]);
	}
    }

    public byte getType() { return MFSTRING; }

    public String[] getValue() { return strings; }
    public String get1Value(int pos) { return strings[pos]; }

    public String toString() {
	String str = "{";
	for (int i = 0; i < strings.length; i++) {
	    if (i != 0) str += ", ";
	    str += "\"" + strings[i] + "\"";
	}
	str += "}";
	return str;
    }
}
