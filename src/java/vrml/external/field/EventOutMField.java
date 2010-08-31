package vrml.external.field;
//JAS import java.util.*;
import vrml.external.field.FieldTypes;
//JAS import vrml.external.Browser;


public class EventOutMField extends EventOut {
  public EventOutMField() {EventType = FieldTypes.UnknownType;}

  // sizeof is the size of an EventOutMF* call.
  int sizeof = 0;


  public int           getSize() {
    return sizeof;
  }
}
