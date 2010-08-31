package vrml.external.exception;

public class InvalidNodeException extends RuntimeException
{
    /**
     * Constructs an InvalidNodeException with no detail message.
     */
    public InvalidNodeException() {
    super();
    }

    /**
     * Constructs an InvalidNodeException with the specified detail message.
     * A detail message is a String that describes this particular exception.
     * @param s the detail message
     */
    public InvalidNodeException(String s) {
     super(s);
    }
}
