package sai;
import org.web3d.x3d.sai.*;

public class FWRoute implements X3DRoute {
	FreeWRLNode sourceNode;
	FreeWRLNode destNode;
	String sourceField;
	String destField;

	public FWRoute(FreeWRLNode sn, String sf, FreeWRLNode dn, String df) {
		sourceNode = sn;
		sourceField = sf;
		destNode = dn;
		destField = df;
	}

	public String toString() {
		String temp;
		temp = "" + sourceNode.getPerlPtr() + " " + sourceField + " " + destNode.getPerlPtr() + " " + destField;
		return temp;
	}

        public boolean equals(Object o) {
                return (o != null) && (o instanceof FWRoute) && (sourceNode.equals(((FWRoute)o).sourceNode)) && (destNode.equals(((FWRoute)o).destNode)) && (sourceField.equals(((FWRoute)o).sourceField)) && (destField.equals(((FWRoute)o).destField));
        }

	public X3DNode getSourceNode() throws InvalidRouteException {
		return sourceNode;
	}

	public X3DNode getDestinationNode() throws InvalidRouteException {
		return destNode;
	}

	public String getSourceField() throws InvalidRouteException {
		return sourceField;
	}

	public String getDestinationField() throws InvalidRouteException {
		return destField;
	}

	public void dispose() {

	}
}
