package sai;
import org.web3d.x3d.sai.*;
import java.util.*;

public class FreeWRLNodeTypes implements X3DNodeTypes {
	private static final HashMap nodeTypes;
	private static final HashMap fwTypes;

 	public int X3D_Component_Networking               = 1;
	public int X3D_Component_Shape                    = 2;
	public int X3D_Component_Geometry2D               = 3;
	public int X3D_Component_Sound                    = 4;
	public int X3D_Component_EnvironmentalEffects     = 5;
	public int X3D_Component_Navigation               = 6;
	public int X3D_Component_EventUtilities           = 7;
	public int X3D_Component_Geometry3D               = 8;
	public int X3D_Component_Rendering                = 9;
	public int X3D_Component_Interpolation            = 10;
	public int X3D_Component_Nurbs                    = 11;
	public int X3D_Component_PointingDevice           = 12;
	public int X3D_Component_Lighting                 = 13;
	public int X3D_Component_Text                     = 14;
	public int X3D_Component_Geospatial               = 15;
	public int X3D_Component_Grouping                 = 16;
	public int X3D_Component_HAnim                    = 17;
	public int X3D_Component_Texturing                = 18;
	public int X3D_Component_EnvironmentalSensor      = 19;
	public int X3D_Component_Scripting                = 20;
	public int X3D_Component_Time                     = 21;


	static {
		nodeTypes = new HashMap();
		nodeTypes.put(new Integer(X3DBoundedObject), "X3DBoundedObject");
		nodeTypes.put(new Integer(X3DBounded2DObject), "X3DBounded2DObject");
		nodeTypes.put(new Integer(X3DURLObject), "X3DURLObject");
		nodeTypes.put(new Integer(X3DAppearanceNode), "X3DAppearnaceNode");
		nodeTypes.put(new Integer(X3DAppearanceChildNode), "X3DAppearanceChildNode");
		nodeTypes.put(new Integer(X3DMaterialNode), "X3DMaterialNode");
		nodeTypes.put(new Integer(X3DTextureNode), "X3DTextureNode");
		nodeTypes.put(new Integer(X3DTexture2DNode), "X3DTexture2DNode");
		nodeTypes.put(new Integer(X3DTexture3DNode), "X3DTexture3DNode");
		nodeTypes.put(new Integer(X3DTextureTransformNode), "X3DTextureTransformNode");
		nodeTypes.put(new Integer(X3DTextureTransform2DNode), "X3DTextureTransform2DNode");
		nodeTypes.put(new Integer(X3DGeometryNode), "X3DGeometryNode");
		nodeTypes.put(new Integer(X3DTextNode), "X3DTextNode");
		nodeTypes.put(new Integer(X3DParametricGeometryNode), "X3DParametricGeometryNode");
		nodeTypes.put(new Integer(X3DGeometricPropertyNode), "X3DGeometricPropertyNode");
		nodeTypes.put(new Integer(X3DColorNode), "X3DColorNode");
		nodeTypes.put(new Integer(X3DCoordinateNode), "X3DCoordinateNode");
		nodeTypes.put(new Integer(X3DNormalNode), "X3DNormalNode");
		nodeTypes.put(new Integer(X3DTextureCoordinateNode), "X3DTextureCoordinateNode");
		nodeTypes.put(new Integer(X3DFontStyleNode), "X3DFontStyleNode");
		nodeTypes.put(new Integer(X3DProtoInstance), "X3DProtoInstance");
		nodeTypes.put(new Integer(X3DChildNode), "X3DChildNode");
		nodeTypes.put(new Integer(X3DBindableNode), "X3DBindanbleNode");
		nodeTypes.put(new Integer(X3DBackgroundNode), "X3DBackgroundNode");
		nodeTypes.put(new Integer(X3DGroupingNode), "X3DGroupingNode");
		nodeTypes.put(new Integer(X3DShapeNode), "X3DShapeNode");
		nodeTypes.put(new Integer(X3DInterpolatorNode), "X3DInterpolatorNode");
		nodeTypes.put(new Integer(X3DLightNode), "X3DLightNode");
		nodeTypes.put(new Integer(X3DScriptNode), "X3DScriptNode");
		nodeTypes.put(new Integer(X3DSensorNode), "X3DSensorNode");
		nodeTypes.put(new Integer(X3DEnvironmentalSensorNode), "X3DEnvironmentalSensorNode");
		nodeTypes.put(new Integer(X3DKeyDeviceSensorNode), "X3DKeyDeviceSensorNode");
		nodeTypes.put(new Integer(X3DNetworkSensorNode), "X3DNetworkSensorNode");
		nodeTypes.put(new Integer(X3DPointingDeviceSensorNode), "X3DPointingDeviceSensorNode");
		nodeTypes.put(new Integer(X3DDragSensorNode), "X3DDragSensorNode");
		nodeTypes.put(new Integer(X3DTouchSensorNode), "X3DTouchSensorNode");
		nodeTypes.put(new Integer(X3DSequencerNode), "X3DSequencerNode");
		nodeTypes.put(new Integer(X3DTimeDependentNode), "X3DTimeDependentNode");
		nodeTypes.put(new Integer(X3DSoundSourceNode), "X3DSoundSourceNode");
		nodeTypes.put(new Integer(X3DTriggerNode), "X3DTriggerNode");
		nodeTypes.put(new Integer(X3DInfoNode), "X3DInfoNode");

		fwTypes = new HashMap();
	}

	public static String getStringType(int type) {
		return (String) nodeTypes.get(new Integer(type));
	}
}
	
