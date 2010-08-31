package vrml.external.exception;

public class InvalidVrmlException extends RuntimeException
{
    /**
     * Constructs an InvalidVrmlException with no detail message.
     */
    public InvalidVrmlException() {
       super();
    }

    /**
     * Constructs an InvalidVrmlException with the specified detail message.
     * A detail message is a String that describes this particular exception.
     * @param s the detail message
     */
    public InvalidVrmlException(String s) {
       super(s);
    }
}
