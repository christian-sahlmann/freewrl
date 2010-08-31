package vrml.external;

import vrml.external.field.*;
import vrml.external.exception.*;
import java.util.*;
//JAS import java.awt.*;
//JAS import java.net.*;
//JAS import java.io.*;
//JAS import java.lang.reflect.*;

public class Node {
  // Get a string specifying the type of this node. May return the
  // name of a PROTO, or the class name


// the following fields are for treating this node as an eventIn or eventOut.
 public int EventType = FieldTypes.UnknownType;
 public String outNode;	// Node to send the command to... NULL if not
			// a get value from viewer call (ie, a Listener
			// response...
 public String inNode;
 public String command;	// the actual command...
 public String RLreturn;	// If this is a register listener response...
 public int nodeptr = 0; //pointer to start of FreeWRL structure in memory
 public int offset = 0;  //offset of actual field in memory from base.
 public int datasize = 0; // how long this data really is
 public String datatype;
 public int ScriptType = 0; // non zero indicates sending to a javascript



  public String        getType() {
	// spec says:
	// Get a string specifying the type of this node. May return the
	// name of a PROTO, or the class name
	// this will return 2 strings, first X3D node, second DEF name, or __UNDEFINED

	String NT = Browser.SendNodeEAIType(nodeptr);
	//System.out.println ("in Java, getType returns " + NT);
	String[] spl = NT.split(" ",0);

	//System.out.println ("string split is " + spl[0] + " and " + spl[1]);
	if (spl[1].equals("__UNDEFINED")) return spl[0];
	return spl[1];
  }


  public EventIn       getEventIn(String name) throws InvalidEventInException {

	EventIn ret;

	String NNN = "nodeFrom_getEventIn";
      StringTokenizer tokens;

  // Return the type that is asked for. To determine the
  // subclass, look at the string.

    String st = Browser.SendEventType(nodeptr, name, "eventIn");

    tokens = new StringTokenizer (st);
    String NNPR = tokens.nextToken();
    String NOFF = tokens.nextToken();
    String NDS = tokens.nextToken();
    String NewDT = tokens.nextToken();
    String ScrT = tokens.nextToken();
    //System.out.println ("EventIn: NNPR " + NNPR + " NOFF " + NOFF +
    //	" NDS " + NDS + " NewDT " + NewDT + " ScrTyp" + ScrT);

    // check out the return values specified in CFuncs/EAIServ.c
    if(NewDT.equals("p")) { ret = new EventInMFString();
    } else if(NewDT.equals("k")) { ret = new EventInSFImage();
    } else if(NewDT.equals("e")) { ret = new EventInSFTime();
    } else if(NewDT.equals("c")) { ret = new EventInSFColor();
    } else if(NewDT.equals("l")) { ret = new EventInMFColor();
    } else if(NewDT.equals("d")) { ret = new EventInSFFloat();
    } else if(NewDT.equals("m")) { ret = new EventInMFFloat();
    } else if(NewDT.equals("o")) { ret = new EventInMFInt32();
    } else if(NewDT.equals("h")) { ret = new EventInSFNode();
    } else if(NewDT.equals("r")) { ret = new EventInMFRotation();
    } else if(NewDT.equals("s")) { ret = new EventInMFVec2f();
    } else if(NewDT.equals("j")) { ret = new EventInSFVec2f();
    } else if(NewDT.equals("t")) { ret = new EventInMFVec3f();
    } else if(NewDT.equals("q")) { ret = new EventInMFNode();
    } else if(NewDT.equals("i")) { ret = new EventInSFRotation();
    } else if(NewDT.equals("g")) { ret = new EventInSFString();
    } else if(NewDT.equals("b")) { ret = new EventInSFBool();
    } else if(NewDT.equals("f")) { ret = new EventInSFInt32();
    } else if(NewDT.equals("u")) { ret = new EventInSFVec3f();
    } else {
	throw new InvalidEventInException("getEventIn - node field error for \"" + name + "\"");
    }

    ret.command = name; ret.inNode = NNN; ret.datatype=NewDT; 
    ret.nodeptr= Integer.parseInt(NNPR); ret.offset=Integer.parseInt(NOFF);
    ret.datasize = Integer.parseInt(NDS); ret.ScriptType = Integer.parseInt(ScrT);
    return ret;
  }


  // Means of getting a handle to an EventOut of this node

  public EventOut      getEventOut(String name) throws InvalidEventOutException {
	EventOut ret;
      StringTokenizer tokens;
	String NNN = "nodeFrom_getEventOut";

    String st = Browser.SendEventType(nodeptr, name, "eventOut");

    tokens = new StringTokenizer (st);
    String NNPR = tokens.nextToken();
    String NOFF = tokens.nextToken();
    String NDS = tokens.nextToken();
    String NewDT = tokens.nextToken();
    String ScrT = tokens.nextToken();
    // System.out.println ("EventOut: NNPR " + NNPR + " NOFF " + NOFF +
    //	" NDS " + NDS + " NewDT " + NewDT + " ScrTyp" + ScrT);



    // check out the return values specified in CFuncs/EAIServ.c
	// #define EAI_SFUNKNOWN           'a'
	// #define EAI_SFBOOL              'b'
	// #define EAI_SFCOLOR             'c'
	// #define EAI_SFFLOAT             'd'
	// #define EAI_SFTIME              'e'
	// #define EAI_SFINT32             'f'
	// #define EAI_SFSTRING            'g'
	// #define EAI_SFNODE              'h'
	// #define EAI_SFROTATION          'i'
	// #define EAI_SFVEC2F             'j'
	// #define EAI_SFIMAGE             'k'
	// #define EAI_MFCOLOR             'l'
	// #define EAI_MFFLOAT             'm'
	// #define EAI_MFTIME              'n'
	// #define EAI_MFINT32             'o'
	// #define EAI_MFSTRING            'p'
	// #define EAI_MFNODE              'q'
	// #define EAI_MFROTATION          'r'
	// #define EAI_MFVEC2F             's'
	// #define EAI_MFVEC3F             't'
	// #define EAI_SFVEC3F             'u'


    if(NewDT.equals("p")) { ret = new EventOutMFString();
    } else if(NewDT.equals("k")) { ret = new EventOutSFImage();
    } else if(NewDT.equals("e")) { ret = new EventOutSFTime();
    } else if(NewDT.equals("c")) { ret = new EventOutSFColor();
    } else if(NewDT.equals("l")) { ret = new EventOutMFColor();
    } else if(NewDT.equals("d")) { ret = new EventOutSFFloat();
    } else if(NewDT.equals("m")) { ret = new EventOutMFFloat();
    } else if(NewDT.equals("o")) { ret = new EventOutMFInt32();
    } else if(NewDT.equals("h")) { ret = new EventOutSFNode();
    } else if(NewDT.equals("r")) { ret = new EventOutMFRotation();
    } else if(NewDT.equals("s")) { ret = new EventOutMFVec2f();
    } else if(NewDT.equals("j")) { ret = new EventOutSFVec2f();
    } else if(NewDT.equals("t")) { ret = new EventOutMFVec3f();
    } else if(NewDT.equals("q")) { ret = new EventOutMFNode();
    } else if(NewDT.equals("i")) { ret = new EventOutSFRotation();
    } else if(NewDT.equals("g")) { ret = new EventOutSFString();
    } else if(NewDT.equals("b")) { ret = new EventOutSFBool();
    } else if(NewDT.equals("f")) { ret = new EventOutSFInt32();
    } else if(NewDT.equals("u")) { ret = new EventOutSFVec3f();
    } else {
	throw new InvalidEventOutException("getEventOut - node field error for field \"" + name + "\"");
    }

    ret.command = name; ret.inNode = NNN; ret.datatype=NewDT; 
	ret.nodeptr=Integer.parseInt(NNPR); ret.offset=Integer.parseInt(NOFF);
    ret.datasize = Integer.parseInt(NDS); ret.ScriptType = Integer.parseInt(ScrT);
    return ret;
  }
}
