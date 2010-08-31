package vrml.external.field;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;
import java.awt.*;
import java.math.BigInteger;

public class EventInSFImage extends EventIn {

  public EventInSFImage() { EventType = FieldTypes.SFIMAGE; }

  public void          setValue(int width, int height, int components, byte[] pixels) throws IllegalArgumentException {
	int count;
	int pixcount;
	String val;
	BigInteger newval;
	byte xx[];


	if (pixels.length != (width*height*components)) {
		throw new IllegalArgumentException();
	}

	if ((components < 1) || (components > 4)) {
		throw new IllegalArgumentException();
	}

	// use BigInt to ensure sign bit does not frick us up.
	xx = new byte[components+1];
	xx[0] = (byte) 0; // no sign bit here!

	val = new String("" + width  + " " + height + " " + components);

	if (pixels== null) { pixcount = 0;} else {pixcount=pixels.length;}

	if (components == 1) {
		for (count = 0; count < pixcount; count++) {
			xx[1] = pixels[count];  
			newval = new BigInteger(xx);
			//System.out.println ("Big int " + newval.toString(16));
			val = val.concat(" 0x" + newval.toString(16));
		}	
	}
	if (components == 2) {
		for (count = 0; count < pixcount; count+=2) {
			xx[1] = pixels[count]; xx[2] = pixels[count+1]; 
			newval = new BigInteger(xx);
			//System.out.println ("Big int " + newval.toString(16));
			val = val.concat(" 0x" + newval.toString(16));

		}	
	}
	if (components == 3) {
		for (count = 0; count < pixcount; count+=3) {
			xx[1] = pixels[count]; xx[2] = pixels[count+1]; xx[3]=pixels[count+2];
			newval = new BigInteger(xx);
			//System.out.println ("Big int " + newval.toString(16));
			val = val.concat(" 0x" + newval.toString(16));
		}	
	}
	if (components == 4) {
		for (count = 0; count < pixcount; count+=4) {
			xx[1] = pixels[count]; xx[2] = pixels[count+1]; xx[3]=pixels[count+2]; xx[4]=pixels[count+3];
			newval = new BigInteger(xx);
			//System.out.println ("Big int " + newval.toString(16));
			val = val.concat(" 0x" + newval.toString(16));

		}	
	}
	//System.out.println ("sending " + val);

    Browser.newSendEvent (this, val.length() + ":" + val + " ");

	return;
  }
}
