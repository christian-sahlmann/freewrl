package vrml;
//JAS import java.util.Hashtable;

public class Browser
{
    Browser() {}
   public String toString() {return "";}   // This overrides a method in Object

   // Browser interface
   public String getName() {
			return "VRML/X3D Browser maintained by Communications Research Centre, Canada" ;
		}
   public String getVersion() {return "0.02";}

   public float getCurrentSpeed() {return (float)0.0;} // XXX

   public float getCurrentFrameRate() {return (float)0.0;} // XXX

   // public String getWorldURL();
   // public void replaceWorld(BaseNode[] nodes);

    public BaseNode[] createVrmlFromString(String vrmlSyntax)
	throws InvalidVRMLSyntaxException
    {
	return FWJavaScript.createVrmlFromString(vrmlSyntax);
    }
   //

   // public void createVrmlFromURL(String[] url, BaseNode node, String event)
   //   throws InvalidVRMLSyntaxException;

   // public void addRoute(BaseNode fromNode, String fromEventOut,
   //                      BaseNode toNode, String toEventIn);

   // public void deleteRoute(BaseNode fromNode, String fromEventOut,
   //                         BaseNode toNode, String toEventIn);

   // public void loadURL(String[] url, String[] parameter)
   //   throws InvalidVRMLSyntaxException;

   // public void setDescription(String description);
}


