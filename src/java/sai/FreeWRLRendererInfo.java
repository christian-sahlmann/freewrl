package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FreeWRLRendererInfo {
	private static HashMap renderingProperties;
	
	static {
		renderingProperties = new HashMap();
		
		renderingProperties.put("Shading", "Phong");
		renderingProperties.put("MaxTextureSize", "1024x1024");
		renderingProperties.put("TextureUnits", new Integer(1));
		renderingProperties.put("AntiAliased", new Boolean(true));
		renderingProperties.put("ColorDepth", new Integer(64));
		renderingProperties.put("TextureMemory", new Float(1024.0));
	}

	public static void setRenderingProperty(String key, Object value) {
		if (!renderingProperties.containsKey(key)) {
			System.out.println("Attempted to add invalid key " + key + "  to rendering Properties table.  Key rejected.");
		} else {
			renderingProperties.remove(key);
			renderingProperties.put(key, value);
		}
	}

	public static Object getRenderingProperty(String key) {
		return renderingProperties.get(key);
	}

	public static Map getRenderingProperties() {
		return renderingProperties;
	}
}
		
