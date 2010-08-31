package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FWProfInfo {
	private static FWProfileInfo profiles[];
	private static int numProfiles = 0;
	private static HashMap profileMap;
        private static final int NUM_COMPONENTS = 24;
	private static final int NUM_PROFILES = 6;
        private static FWComponentInfo components[];
        private static int numComponents = 24;
        private static HashMap componentMap;
	private static ProfileInfo temp;
	private static ComponentInfo temp2;

	static {
		System.out.println("in static intializer");
                componentMap = new HashMap();
		components = new FWComponentInfo[NUM_COMPONENTS];
                components[0] = new FWComponentInfo("Core", 2, "Core component", "http://www.crc.ca/freewrl");
                componentMap.put("Core", components[0]);
                components[1] = new FWComponentInfo("DIS", 1, "DIS component", "http://www.crc.ca/freewrl");
                componentMap.put("DIS", components[1]);
                components[2] = new FWComponentInfo("EnvironmentalEffects", 3, "Environmental Effects component", "http://www.crc.ca/freewrl");
                componentMap.put("EnvironmentalEffects", components[2]);
                components[3] = new FWComponentInfo("EnvironmentalSensor", 2, "Environmental Sensor component", "http://www.crc.ca/freewrl");
                componentMap.put("EnvironmentalSensor", components[3]);
                components[4] = new FWComponentInfo("Geometry2D", 2, "2D Geometry component", "http://www.crc.ca/freewrl");
                componentMap.put("Geometry2D", components[4]);
                components[5] = new FWComponentInfo("Geometry3D", 4, "3D Geometry component", "http://www.crc.ca/freewrl");
                componentMap.put("Geometry3D", components[5]);
                components[6] = new FWComponentInfo("Geospatial", 1, "Geospatial Rendering component", "http://www.crc.ca/freewrl");
                componentMap.put("Geospatial", components[6]);
                components[7] = new FWComponentInfo("Grouping", 3, "Grouping component", "http://www.crc.ca/freewrl");
                componentMap.put("Grouping", components[7]);
                components[8] = new FWComponentInfo("H-Anim", 1, "Humanoid Animation component", "http://www.crc.ca/freewrl");
                componentMap.put("H-Anim", components[8]);
                components[9] = new FWComponentInfo("Interpolation", 3, "Interpolation component", "http://www.crc.ca/freewrl");
                componentMap.put("Interpolation", components[9]);
                components[10] = new FWComponentInfo("KeyDeviceSensor", 2, "Key Device Sensor component", "http://www.crc.ca/freewrl");
                componentMap.put("KeyDeviceSensor", components[10]);
                components[11] = new FWComponentInfo("Lighting", 3, "Lighting component", "http://www.crc.ca/freewrl");
                componentMap.put("Lighting", components[11]);
                components[12] = new FWComponentInfo("Navigation", 2, "Navigation component", "http://www.crc.ca/freewrl");
                componentMap.put("Navigation", components[12]);
                components[13] = new FWComponentInfo("Networking", 3, "Networking component", "http://www.crc.ca/freewrl");
                componentMap.put("Networking", components[13]);
                components[14] = new FWComponentInfo("NURBS", 4, "NURBS component", "http://www.crc.ca/freewrl");
                componentMap.put("NURBS", components[14]);
                components[15] = new FWComponentInfo("PointingDeviceSensor", 2, "Pointing Device Sensor component", "http://www.crc.ca/freewrl");
                componentMap.put("PointingDeviceSensor", components[15]);
                components[16] = new FWComponentInfo("Rendering", 4, "Rendering component", "http://www.crc.ca/freewrl");
                componentMap.put("Rendering", components[16]);
                components[17] = new FWComponentInfo("Scripting", 1, "Scripting component", "http://www.crc.ca/freewrl");
                componentMap.put("Scripting", components[17]);
                components[18] = new FWComponentInfo("Shape", 3, "Shape component", "http://www.crc.ca/freewrl");
                componentMap.put("Shape", components[18]);
                components[19] = new FWComponentInfo("Sound", 1, "Sound component", "http://www.crc.ca/freewrl");
                componentMap.put("Sound", components[19]);
                components[20] = new FWComponentInfo("Text", 1, "Text component", "http://www.crc.ca/freewrl");
                componentMap.put("Text", components[20]);
                components[21] = new FWComponentInfo("Texturing", 3, "Texturing component", "http://www.crc.ca/freewrl");
                componentMap.put("Texturing", components[21]);
                components[22] = new FWComponentInfo("Time", 2, "Time component", "http://www.crc.ca/freewrl");
                componentMap.put("Time", components[22]);
                components[23] = new FWComponentInfo("EventUtilities", 1, "Event Utilities component", "http://www.crc.ca/freewrl");
                componentMap.put("EventUtilities", components[23]);

		System.out.println("finished component init");
		// set static profile info
		profileMap = new HashMap();
		profiles = new FWProfileInfo[NUM_PROFILES];
		try {
			FWComponentInfo[] components = new FWComponentInfo[1];
			components[0] = getComponent("Core", 1);
			profiles[numProfiles] = new FWProfileInfo("Core", "Core Profile", components);
			profileMap.put("Core", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}
		System.out.println("finished Core init");
		try {
			FWComponentInfo[] components = new FWComponentInfo[12];
			components[0] = getComponent("Core", 1);
			components[1] = getComponent("Time", 1);
			components[2] = getComponent("Networking", 1);
			components[3] = getComponent("Grouping", 1);
			components[4] = getComponent("Rendering", 3);
			components[5] = getComponent("Shape", 1);
			components[6] = getComponent("Geometry3D", 2);
			components[7] = getComponent("Lighting", 1);
			components[8] = getComponent("Texturing", 2);
			components[9] = getComponent("Interpolation", 2);
			components[10] = getComponent("Navigation", 1);
			components[11] = getComponent("EnvironmentalEffects", 1);
			profiles[numProfiles] = new FWProfileInfo("Interchange", "Interchange profile", components);
			profileMap.put("Interchange", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}
		System.out.println("finished Interchange init");
		try {
			FWComponentInfo[] components = new FWComponentInfo[16];
			components[0] = getComponent("Core", 1);
			components[1] = getComponent("Time", 1);
			components[2] = getComponent("Networking", 1);
			components[3] = getComponent("Grouping", 2);
			components[4] = getComponent("Rendering", 2);
			components[5] = getComponent("Shape", 1);
			components[6] = getComponent("Geometry3D", 3);
			components[7] = getComponent("Lighting", 2);
			components[8] = getComponent("Texturing", 2);
			components[9] = getComponent("Interpolation", 2);
			components[10] = getComponent("Navigation", 1);
			components[11] = getComponent("EnvironmentalEffects", 1);
			components[12] = getComponent("PointingDeviceSensor", 1);
			components[13] = getComponent("KeyDeviceSensor", 1);
			components[14] = getComponent("EnvironmentalSensor", 1);
			components[15] = getComponent("EventUtilities", 1);
			profiles[numProfiles] = new FWProfileInfo("Interactive", "Interactive profile", components);
			profileMap.put("Interactive", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}
		try {
			FWComponentInfo[] components = new FWComponentInfo[14];
			components[0] = getComponent("Core", 1);
			components[1] = getComponent("Time", 1);
			components[2] = getComponent("Networking", 2);
			components[3] = getComponent("Grouping", 2);
			components[4] = getComponent("Rendering", 1);
			components[5] = getComponent("Shape", 1);
			components[6] = getComponent("Geometry3D", 2);
			components[7] = getComponent("Lighting", 2);
			components[8] = getComponent("Texturing", 1);
			components[9] = getComponent("Interpolation", 2);
			components[10] = getComponent("Navigation", 1);
			components[11] = getComponent("EnvironmentalEffects", 1);
			components[12] = getComponent("PointingDeviceSensor", 1);
			components[13] = getComponent("EnvironmentalSensor", 1);
			profiles[numProfiles] = new FWProfileInfo("MPEG-4", "MPEG-4 Interactive profile", components);
			profileMap.put("MPEG-4", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}
		try {
			FWComponentInfo[] components = new FWComponentInfo[20];
			components[0] = getComponent("Core", 2);
			components[1] = getComponent("Time", 1);
			components[2] = getComponent("Networking", 3);
			components[3] = getComponent("Grouping", 2);
			components[4] = getComponent("Rendering", 3);
			components[5] = getComponent("Shape", 2);
			components[6] = getComponent("Geometry3D", 4);
			components[7] = getComponent("Lighting", 2);
			components[8] = getComponent("Texturing", 3);
			components[9] = getComponent("Interpolation", 2);
			components[10] = getComponent("Navigation", 2);
			components[11] = getComponent("EnvironmentalEffects", 2);
			components[12] = getComponent("PointingDeviceSensor", 1);
			components[13] = getComponent("KeyDeviceSensor", 2);
			components[14] = getComponent("EnvironmentalSensor", 2);
			components[15] = getComponent("EventUtilities", 1);
			components[16] = getComponent("Geometry2D", 1);
			components[17] = getComponent("Text", 1);
			components[18] = getComponent("Sound", 1);
			components[19] = getComponent("Scripting", 1);
			profiles[numProfiles] = new FWProfileInfo("Immersive", "Immersive profile", components);
			profileMap.put("Immersive", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}
		try {
			FWComponentInfo[] components = new FWComponentInfo[24];
			components[0] = getComponent("Core", 2);
			components[1] = getComponent("Time", 2);
			components[2] = getComponent("Networking", 3);
			components[3] = getComponent("Grouping", 2);
			components[4] = getComponent("Rendering", 4);
			components[5] = getComponent("Shape", 3);
			components[6] = getComponent("Geometry3D", 4);
			components[7] = getComponent("Lighting", 3);
			components[8] = getComponent("Texturing", 3);
			components[9] = getComponent("Interpolation", 3);
			components[10] = getComponent("Navigation", 2);
			components[11] = getComponent("EnvironmentalEffects", 3);
			components[12] = getComponent("PointingDeviceSensor", 1);
			components[13] = getComponent("KeyDeviceSensor", 2);
			components[14] = getComponent("EnvironmentalSensor", 2);
			components[15] = getComponent("EventUtilities", 1);
			components[16] = getComponent("Geometry2D", 2);
			components[17] = getComponent("Text", 1);
			components[18] = getComponent("Sound", 1);
			components[19] = getComponent("Scripting", 1);
			components[20] = getComponent("Geospatial", 1);
			components[21] = getComponent("H-Anim", 1);
			components[22] = getComponent("NURBS", 4);
			components[23] = getComponent("DIS", 1);
			profiles[numProfiles] = new FWProfileInfo("Full", "Full profile", components);
			profileMap.put("Full", profiles[numProfiles]);
			numProfiles++;
		} catch (Exception e) {
			System.out.println(e);
		}

	}

	public static FWProfileInfo getProfile(String name) throws NotSupportedException {
		temp = (FWProfileInfo) profileMap.get(name);
		if (temp  == null) {
			throw new NotSupportedException("The profile " + name + " is not currently supported");
		} else {
			return (FWProfileInfo) temp; 
		}

	}

	public static FWProfileInfo[] getProfiles() {
		return profiles;
	}
        public static ComponentInfo[] getComponents() {
                return components;
        }

        public static FWComponentInfo getComponent(String name, int level) throws NotSupportedException {
                FWComponentInfo temp;

                temp = (FWComponentInfo) componentMap.get(name);
                if (temp == null) {
                        throw new NotSupportedException("Component " + name + " at level " + level + " is not supported");
                } else if (temp.getLevel() >= level) {
                        return temp;
                } else {
                        throw new NotSupportedException("Component " + name + " at level " + level + " is not supported");
                }
        }
}	
