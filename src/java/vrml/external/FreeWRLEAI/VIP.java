// copyright (c) 1997,1998 stephen f. white
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
package vrml.external.FreeWRLEAI;
//JAS import vrml.external.FreeWRLEAI.*;

public final class VIP {
    public static final short	QUIT = -1;
    public static final short	MESSAGE = -2;
    public static final short	ADD_OBJECT = -3;
    public static final short	REMOVE_OBJECT = -4;
    public static final short	PRIVATE_MESSAGE = -5;
    public static final short	CREATE_OBJECT = -6;
    public static final short	USER_INFO = -7;
    public static final short  SELF_INFO = -8;
    public static final short  SSRC = -9;
    public static final short  TRANSFERREQUEST = -10;
    public static final short  TRANSFERACCEPT = -11;
    public static final short  TRANSFERREJECT = -12;
    public static final short  TRANSFERREQUESTADD = -13;
    public static final short  FILEREQUEST = -14;
    public static final short  FRQRESPONSE = -15;

    public static final short	POSITION = 0;
    public static final short	ORIENTATION = 1;
    public static final short	SCALE = 2;
    public static final short	NAME = 3;
    public static final short  OWNER = 4;
    public static final short  PARENT = 5;
    public static final short  CHILDREN = 6;
    public static final short  DROPPED = 7;


    // this is the number of fields reserved by the VIP protocol
    public static final short	NUM_FIELDS = 4;

   // this is the maximum number of possible gestures
   public static final short MAX_GESTURES = 10;

   // this is the maximum number of children
   public static final short MAX_CHILDREN = 50;

    public static String fieldName(short value) {
        switch (value) {
	  case QUIT:		return "QUIT";
	  case MESSAGE:		return "message";
	  case ADD_OBJECT:	return "add_object";
	  case REMOVE_OBJECT:	return "remove_object";
	  case PRIVATE_MESSAGE:	return "private_message";
	  case CREATE_OBJECT:	return "create_object";
	  case USER_INFO:	return "user_info";

	  case POSITION:	return "position";
	  case ORIENTATION:	return "orientation";
	  case SCALE:		return "scale";
	  case NAME:		return "name";
	  default:		return String.valueOf(value);
	}
    }

    static String msgToString(int vid, short field, VField value) {
	return vid + " " + fieldName(field) + " " + value;
    }
}
