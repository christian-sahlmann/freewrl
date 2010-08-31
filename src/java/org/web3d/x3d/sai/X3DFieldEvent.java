package org.web3d.x3d.sai;
import java.util.*;

public class X3DFieldEvent extends EventObject {
        Object data;
        double time;
        Object source;

        public X3DFieldEvent(Object src, double t, Object d) {
		super(src);
                data = d;
                time = t;
                source = src;
        }

        public double getTime() {
                return time;
        }

        public Object getData() {
                return data;
        }
}
