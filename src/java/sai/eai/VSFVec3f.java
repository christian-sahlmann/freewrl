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

public class VSFVec3f extends VField
{
    private float[]	values = new float[3];

    public VSFVec3f(float x, float y, float z)
    {
	values[0] = x;
	values[1] = y;
	values[2] = z;
    }

    public VSFVec3f(float[] values)
    {
        if (values.length != 3) {
            this.values[0] = values[0];
            this.values[1] = values[1];
            this.values[2] = values[2];
        } else {
            this.values = values;
	}
    }

    public VSFVec3f(DataInputStream in) throws IOException
    {
	values[0] = in.readFloat();
	values[1] = in.readFloat();
	values[2] = in.readFloat();
    }

    public void write(DataOutputStream out) throws IOException
    {
	out.writeFloat(values[0]);
	out.writeFloat(values[1]);
	out.writeFloat(values[2]);
    }

    public String toString()
    {
	return "(" + values[0] + ", " + values[1] + ", " + values[2] + ")";
    }

    public byte getType() { return SFVEC3F; }

    public float[] getValue() { return values; }

    public VSFVec3f plus(VSFVec3f v) {
	return new VSFVec3f(values[0] + v.values[0],
			    values[1] + v.values[1],
			    values[2] + v.values[2]);
    }

    public VSFVec3f minus(VSFVec3f v) {
	return new VSFVec3f(values[0] - v.values[0],
			    values[1] - v.values[1],
			    values[2] - v.values[2]);
    }

    public VSFVec3f times(float s) {
	return new VSFVec3f(values[0] * s,
			    values[1] * s,
			    values[2] * s);
    }

    /* Isabelle April 8 1999 for proximity calculation */

    public double getDistance(VSFVec3f v) {

        double x;
        double y;
        double z;
        double distance;

        x = (double)(values[0] - v.values[0]);
        y = (double)(values[1] - v.values[1]);
        z = (double)(values[2] - v.values[2]);

        distance = Math.sqrt((x*x) + (y*y) + (z*z));
        return distance;
    }

    public double getAngle(VSFVec3f v) {

	/* working on the x-z plan for now.  I might add the y axis later on */
	double delta_x;
	double delta_z;
	double angle;  /* in radians */

	delta_x = (double) (values[0] - v.values[0]);
	/* axis shifted 180 degrees with standard.  Therefore need to invert the parameters */
	delta_z = (double) (v.values[2] - values[2]);
	angle = Math.atan2(delta_z, delta_x);  /* returns the angle whose tangent is z/x */
	return(angle);

    }


}
