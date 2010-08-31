// the type number identificators are in CFuncs/headers.h
package vrml;
import vrml.field.*;

public class FWCreateField {

    public static Field createField(String type) {
	type = type.intern();

        if (type =="1") return new SFBool();
        else if (type =="2") return new SFColor();
        else if (type =="3") return new SFFloat();
        else if (type =="4") return new SFTime();
        else if (type =="5") return new SFInt32();
        else if (type =="6") return new SFString();
        else if (type =="7") return new SFNode();
        else if (type =="8") return new SFRotation();
        else if (type =="9") return new SFVec2f();
        else if (type =="10") return new SFImage();
        else if (type =="11") return new MFColor();
        else if (type =="12") return new MFFloat();
        else if (type =="13") return new MFTime();
        else if (type =="14") return new MFInt32();
        else if (type =="15") return new MFString();
        else if (type =="16") return new MFNode();
        else if (type =="17") return new MFRotation();
        else if (type =="18") return new MFVec2f();
        else if (type =="19") return new MFVec3f();
        else if (type =="20") return new SFVec3f();
        else
            throw new IllegalArgumentException("Unknown field type "+type);
    }

    public static ConstField createConstField(String type) {

	type = type.intern();
        if (type =="1") return new ConstSFBool();
        else if (type =="2") return new ConstSFColor();
        else if (type =="3") return new ConstSFFloat();
        else if (type =="4") return new ConstSFTime();
        else if (type =="5") return new ConstSFInt32();
        else if (type =="6") return new ConstSFString();
        else if (type =="7") return new ConstSFNode();
        else if (type =="8") return new ConstSFRotation();
        else if (type =="9") return new ConstSFVec2f();
        else if (type =="10") return new ConstSFImage();
        else if (type =="11") return new ConstMFColor();
        else if (type =="12") return new ConstMFFloat();
        else if (type =="13") return new ConstMFTime();
        else if (type =="14") return new ConstMFInt32();
        else if (type =="15") return new ConstMFString();
        else if (type =="16") return new ConstMFNode();
        else if (type =="17") return new ConstMFRotation();
        else if (type =="18") return new ConstMFVec2f();
        else if (type =="19") return new ConstMFVec3f();
        else if (type =="20") return new ConstSFVec3f();
        else
            throw new IllegalArgumentException("Unknown field type "+type);
    }
}
