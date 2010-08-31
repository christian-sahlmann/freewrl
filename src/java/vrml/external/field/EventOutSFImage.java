package vrml.external.field;
import java.math.BigInteger;
import java.util.*;
import vrml.external.field.FieldTypes;
import vrml.external.Browser;

public class EventOutSFImage extends EventOut {
  public EventOutSFImage() {EventType = FieldTypes.SFIMAGE;}

	int width;
	int height;
	int numComponents;
	StringTokenizer tokens;

	private void takeapart() {
		String rep;

		if (RLreturn == null) {
			rep = Browser.SendEventOut (nodeptr, offset, datasize, datatype, command);
                        if (rep.length() > 2) {
                                // remove quotes at the beginning and end
                                rep = rep.substring (1,rep.length()-1);
                        }

			// System.out.println ("EventOutSFImage - command " + command + " rep " + rep);
			tokens = new StringTokenizer (rep);
		} else {
			// System.out.println ("EventOutSFImage - command " + command + " RLreturn " + RLreturn);
			tokens = new StringTokenizer (RLreturn);
		}

		width = Integer.valueOf(tokens.nextToken()).intValue();
		height = Integer.valueOf(tokens.nextToken()).intValue();
		numComponents = Integer.valueOf(tokens.nextToken()).intValue();
	}

	public int getWidth() {
		takeapart();
		return width;
	}

	public int getHeight () {
		takeapart();
		return height;
	}

	public int getNumComponents() {
		takeapart();
		return numComponents;
	}

	public byte[] getPixels () {
		takeapart();
		byte retval[];
		int count;
		BigInteger nextVal;
		BigInteger bigTmp;

		String nextStr;
		int byteptr;
		int tmp;

		retval = new byte [width*height*numComponents];
		byteptr = 0;

		// loop through the string return vals, and make up byte array 
		for (count = 0; count < width*height; count++) {
			nextStr = tokens.nextToken();

			// is this a hex string?
			try {
				nextVal = new BigInteger(nextStr);
			} catch (Exception e) {
				nextStr = nextStr.substring (2,nextStr.length());
				nextVal = new BigInteger(nextStr,16);
			}

			if (numComponents == 1) {
				tmp = nextVal.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;
			}
			if (numComponents == 2) {
				bigTmp = nextVal.shiftRight(8);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				tmp = nextVal.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;
			}
			if (numComponents == 3) {
				bigTmp = nextVal.shiftRight(16);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				bigTmp = nextVal.shiftRight(8);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				tmp = nextVal.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;
			}
			if (numComponents == 4) {
				bigTmp = nextVal.shiftRight(24);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				bigTmp = nextVal.shiftRight(16);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				bigTmp = nextVal.shiftRight(8);
				tmp = bigTmp.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;

				tmp = nextVal.intValue();
				tmp = tmp & 0xff;
				retval[byteptr]  =(byte) (tmp); byteptr++;
			}
		}

		return retval;
	}
}
