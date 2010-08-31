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

public class VSFRotation extends VField
{
    private float[]	values = new float[4];

    public VSFRotation(float axisX, float axisY, float axisZ, float angle)
    {
	values[0] = axisX;
	values[1] = axisY;
	values[2] = axisZ;
	values[3] = angle;
    }

    public VSFRotation(float[] values)
    {
	if (values.length != 4) {
	    this.values[0] = values[0];
	    this.values[1] = values[1];
	    this.values[2] = values[2];
	    this.values[3] = values[3];
	} else {
	    this.values = values;
	}
    }

    public VSFRotation(DataInputStream in) throws IOException
    {
	values[0] = in.readFloat();
	values[1] = in.readFloat();
	values[2] = in.readFloat();
	values[3] = in.readFloat();
    }

    public void write(DataOutputStream out) throws IOException
    {
	out.writeFloat(values[0]);
	out.writeFloat(values[1]);
	out.writeFloat(values[2]);
	out.writeFloat(values[3]);
    }

    public String toString()
    {
	return "(" + values[0] + ", " + values[1] + ", " + values[2] + ", " + values[3] + ")";
    }

    public byte getType() { return SFROTATION; }

    public float[] getValue() {
	return values;
    }

    /* Isabelle June 25 1999 for RAT 3D */

    public double getAngle() {
	/* a 90 degree correcting factor is required to map VNet initial orientation to */
	/* standard convention.  VNet defines 0 degree as where the avatar is facing */
	/* whereas normally, 0 degree is on the x axis of a circle */

	double angle;

	angle = (double)values[3] + (Math.PI/2);
	if (angle > Math.PI)  /* value can only be between +/- PI */
	{
		angle = angle - (2*Math.PI);
	}
	return angle;
    }

}
