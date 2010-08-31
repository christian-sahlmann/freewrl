package vrml;
import java.io.UnsupportedEncodingException;

public class FWHelper {
    static String base64alphabet =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.";
    static byte[] revBase64;
    static {
	revBase64 = new byte['z'+1];
	for (int i = 0; i < 64; i++) {
	    revBase64[base64alphabet.charAt(i)] = (byte) i;
	}
    }

    public static String base64encode(String str) {
	byte[] utf8;
	try {
	    utf8 = str.getBytes("UTF-8");
	} catch (UnsupportedEncodingException ex) {
	    throw new InternalError(ex.toString());
	}
	StringBuffer sb = new StringBuffer((utf8.length+2)/3 * 4);
	int i;
	int len = utf8.length;
	for (i = 0; i < len - 2; i += 3) {
	    int value = (utf8[i  ] & 0xff) << 16
		| (utf8[i+1] & 0xff) << 8
		| (utf8[i+2] & 0xff);
	    sb.append(base64alphabet.charAt(value >> 18))
		.append(base64alphabet.charAt((value >> 12) & 0x3f))
		.append(base64alphabet.charAt((value >> 6) & 0x3f))
		.append(base64alphabet.charAt(value & 0x3f));
	}
	if (i < len) {
	    int value = utf8[i++] & 0xff;
	    sb.append(base64alphabet.charAt(value >> 2));
	    if (i < len) {
		value = (value << 8) + (utf8[i++] & 0xff);
		sb.append(base64alphabet.charAt((value >> 4) & 0x3f));
		sb.append(base64alphabet.charAt((value << 2) & 0x3f));
	    } else {
		sb.append(base64alphabet.charAt((value << 4) & 0x3f));
		sb.append('=');
	    }
	    sb.append('=');
	}
	return sb.toString();
    }

    public static String base64decode(String str) {
	int len = str.length();
	if ((len & 3) != 0)
	    throw new IllegalArgumentException("Not Base64: "+str);
	int padstart = str.indexOf('=', len - 3);
	int padding = padstart == -1 ? 0 : len - padstart;
	byte[] utf8 = new byte[len / 4 * 3];

	for (int i = 0; i < len / 4; i++) {
	    int value = (revBase64[str.charAt(4*i)] << 18)
		+ (revBase64[str.charAt(4*i+1)] << 12)
		+ (revBase64[str.charAt(4*i+2)] << 6)
		+ revBase64[str.charAt(4*i+3)];
	    utf8[3*i+0] = (byte) (value >> 16);
	    utf8[3*i+1] = (byte) (value >>  8);
	    utf8[3*i+2] = (byte) (value      );
	}
	try {
	    return new String(utf8, 0, utf8.length - padding, "UTF-8");
	} catch (UnsupportedEncodingException ex) {
	    throw new InternalError(ex.toString());
	}
    }

    /**
     * This is the static method, that quotes a string.
     */
    public static String quote(String str) {
        StringBuffer result = new StringBuffer("\"");
        for (int i=0; i< str.length(); i++) {
            char c;
            switch (c = str.charAt(i)) {
            case '\0':
                result.append("\\0");
                break;
            case '\\':
                result.append("\\\\");
                break;
            case '\"':
                result.append("\\\"");
                break;
            default:
		result.append(str.charAt(i));
            }
        }
        return result.append("\"").toString();
    }

    public static String nodeToString(BaseNode node) {
	return node.getType() + "{" /*XXX*/ + "}";
    }
}

