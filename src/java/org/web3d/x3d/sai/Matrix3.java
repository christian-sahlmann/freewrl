package org.web3d.x3d.sai;

public class Matrix3 {
	public float[][] matrix;
	public static int SIZE = 3;
	public Matrix3() {
		int i;
		int j;
		matrix = new float[SIZE][SIZE];
		for (i = 0; i < SIZE; i++) {
			for (j=0; j<SIZE; j++) {
				matrix[i][j] = 0;
			}
		}
	}

	public Matrix3(float[] init) {
		int i;
		int j;
		int count;

		if (init.length < SIZE*SIZE) {
			throw new ArrayIndexOutOfBoundsException("Initialization array passed to Matrix3 of insufficient length");
		}
	
		matrix = new float[SIZE][SIZE];
		count = 0;
		
		for (i = 0; i < SIZE; i++) {
			for (j = 0; j < SIZE; j++) {
				matrix[i][j] = init[count];
				count++;
			}
		}
	}
		
	public void setIdentity() {
		int i, j;

		for (i = 0; i < SIZE; i++) {
			for (j = 0; j < SIZE; j++) {
				if (i==j) {
					matrix[i][j] = 1.0F;
				} else {
					matrix[i][j] = 0.0F;
				}
			}
		}
	}

	public void set(int row, int column, float value) {
		if ((row > SIZE) || (row < 0) || (column > SIZE) || (column < 0)) {
			throw new ArrayIndexOutOfBoundsException("Matrix 3 set passed invalid row or column value");
		}

		matrix[row][column] = value;
	}
	public float get(int row, int column){
		if ((row > SIZE) || (row < 0) || (column > SIZE) || (column < 0)) {
			throw new ArrayIndexOutOfBoundsException("Matrix 3 set passed invalid row or column value");
		}
		
		return matrix[row][column];
	}

	public void setTransform(SFVec2f translation, SFVec3f rotation, SFVec2f scale, SFVec3f scaleOrientation, SFVec2f centre) {
                float[][] rot;
                float[][] finalscale;
                float[][] srot;
                float[][] rrot;
                float[][] nsrot;
                float[][] strans;
                float[][] rtrans;
                float[] axisr;
                float[][] trans;
                float[][] nstrans;
                float[][] sc;
                float[] value;
                int i,j;
                float length;
                float x,y,z;
                float c, s, t;

                this.setIdentity();

                // Change the rotation into a matrix
                rot = new float[SIZE][SIZE];
                srot = new float[SIZE][SIZE];
                rrot = new float[SIZE][SIZE];
                nsrot = new float[SIZE][SIZE];
                trans = new float[SIZE][SIZE];
                strans = new float[SIZE][SIZE];
                rtrans = new float[SIZE][SIZE];
                nstrans = new float[SIZE][SIZE];
                sc = new float[SIZE][SIZE];
                finalscale = new float[SIZE][SIZE];

                value = new float[SIZE];
                axisr = new float[SIZE];

                for (i = 0; i < SIZE; i++) {
                        for (j=0; j < SIZE; j++) {
                                if (i == j) {
                                        rot[i][j] = 1.0F;
                                        srot[i][j] = 1.0F;
                                        rrot[i][j] = 1.0F;
                                        nsrot[i][j] = 1.0F;
                                        trans[i][j] = 1.0F;
                                        strans[i][j] = 1.0F;
                                        rtrans[i][j] = 1.0F;
                                        nstrans[i][j] = 1.0F;
                                        sc[i][j] = 1.0F;
                                        finalscale[i][j] = 1.0F;
                                } else {
                                        trans[i][j] = 0.0F;
                                        strans[i][j] = 0.0F;
                                        rtrans[i][j] = 0.0F;
                                        nstrans[i][j] = 0.0F;
                                        sc[i][j] = 0.0F;
                                        rot[i][j] = 0.0F;
                                        srot[i][j] = 0.0F;
                                        rrot[i][j] = 0.0F;
                                        nsrot[i][j] = 0.0F;
                                        finalscale[i][j] = 0.0F;
                                }
                        }
                }

                if (translation != null) {
                        try {
                                translation.getValue(value);
                        } catch (Exception e) {
                                System.out.println(e);
                        }

                        trans[2][0] = value[0];
                        trans[2][1] = value[1];
                }

                if (scale != null) {
                        try {
                                scale.getValue(value);
                        } catch (Exception e) {
                                System.out.println(e);
                        }

                        sc[0][0] = value[0];
                        sc[1][1] = value[1];

                        if (centre != null) {
                                try {
                                        centre.getValue(value);
                                } catch (Exception e) {
                                        System.out.println(e);
                                }

                                strans[3][0] = value[0];
                                strans[3][1] = value[1];

                        }

                        if (scaleOrientation!=null) {
                                try {
                                        scaleOrientation.getValue(axisr);
                                } catch (Exception e) {
                                        System.out.println(e);
                                }


	                        rtrans[2][0] = -1*axisr[0];
	                        rtrans[2][1] = -1*axisr[1];
	
        	                rrot[0][0] = (float)(Math.cos(axisr[2]));
        	                rrot[0][1] = (float)(Math.sin(axisr[2]));
        	                rrot[0][0] = (float)(-1.0*Math.sin(axisr[2]));
        	                rrot[0][0] = (float)(Math.cos(axisr[2]));

        	                srot = multiply(rtrans, rrot);

                	        rtrans[2][0] = axisr[0];
                	        rtrans[2][1] = axisr[1];

                	        srot = multiply(rot, rtrans);


                        }

                        for (i = 0; i < SIZE; i++) {
                                for (j=0; j< SIZE; j++) {
                                        nsrot[i][j] *= -1.0F;
                                        nstrans[i][j] *= -1.0F;
                                }
                        }

                }


                if (rotation != null) {

                        try {
                                rotation.getValue(axisr);
                        } catch (Exception e) {
                                System.out.println(e);
                        }

			rtrans[2][0] = -1*axisr[0];
			rtrans[2][1] = -1*axisr[1];

			rrot[0][0] = (float)(Math.cos(axisr[2]));
			rrot[0][1] = (float)(Math.sin(axisr[2]));
			rrot[0][0] = (float)(-1.0*Math.sin(axisr[2]));
			rrot[0][0] = (float)(Math.cos(axisr[2]));

			rot = multiply(rtrans, rrot);

			rtrans[2][0] = axisr[0];
			rtrans[2][1] = axisr[1];

			rot = multiply(rot, rtrans);

                }

                matrix = multiply(matrix, trans);
                matrix = multiply(matrix, strans);
                matrix = multiply(matrix, rot);
                matrix = multiply(matrix, srot);
                matrix = multiply(matrix, sc);
                matrix = multiply(matrix, nsrot);
                matrix = multiply(matrix, nstrans);

	}
	public void getTransform (SFVec2f translation, SFVec3f rotation, SFVec2f scale) {
               float[] t;
                float[] s;
                float[] r;

                r = new float[SIZE];
                s = new float[SIZE];
                t = new float[SIZE];

                t[0] = matrix[3][0];
                t[1] = matrix[3][1];

                translation.setValue(t);

                // Note that this only works if the scale transform was applied first - since that is how transforms are applied in VRML, and this is a much
                // more efficient way of doing things we make that assumption

                s[0]  = (float) (Math.sqrt(matrix[0][0]*matrix[0][0] + matrix[1][0]*matrix[1][0] + matrix[2][0]*matrix[2][0] + matrix[3][0]*matrix[3][0]));
                s[1]  = (float) (Math.sqrt(matrix[0][1]*matrix[0][1] + matrix[1][1]*matrix[1][1] + matrix[2][1]*matrix[2][1] + matrix[3][1]*matrix[3][1]));

                scale.setValue(s);

		float angle;

		angle = (float)(Math.acos(matrix[0][0]));

		r[0] = 0.0F;
		r[1] = 0.0F;
		r[2] = angle;

		rotation.setValue(r);
	}

        public float[][] multiply(float[][] multp, float[][] mat) {
                int i, j, k;
                float sum;
                float[][] ans;

                ans = new float[SIZE][SIZE];

                for (i = 0; i < SIZE; i++) {
                        for (j=0; j < SIZE; j++) {
                                sum = 0.0F;
                                for (k = 0; k < SIZE; k++) {
                                        sum = sum + multp[i][k]*mat[k][j];
                                }
                                ans[i][j] = sum;
                        }
                }

                return ans;
        }

	public Matrix3 inverse() {
		float A;
		float a,b,c,d,e,f,g,h,i;
		float[] inverse;
		
		a = matrix[0][0];
		b = matrix[0][1];
		c = matrix[0][2];
		d = matrix[1][0];
		e = matrix[1][1];
		f = matrix[1][2];
		g = matrix[2][0];
		h = matrix[2][1];
		i = matrix[2][2];

		inverse = new float[9];

		A = (a*(e*i-f*h) - b*(d*i-f*g) + c*(d*h - e*g));

		if (A == 0) {
			return null;
		} else {
			A = 1.0F/A;
		}

		inverse[0] = A*(e*i - f*h);
		inverse[1] = A*(c*h - b*i);
		inverse[2] = A*(b*f - c*e);
		inverse[3] = A*(f*g - d*i);
		inverse[4] = A*(a*i - c*g);
		inverse[5] = A*(c*d - a*f);
		inverse[6] = A*(d*h - e*g);
		inverse[7] = A*(b*g - a*h);
		inverse[8] = A*(a*e - b*d);

		return new Matrix3(inverse);
	}
		
	
	public Matrix3 transpose() {
		float[] transp;
		int i, j;

		transp = new float[9];

		for (i = 0; i < SIZE; i++) {
			for (j = 0; j < SIZE; j++) {
				transp[j*SIZE + i] = matrix[i][j];
			}
		}

		return new Matrix3(transp);
	} 

	public Matrix3 multiplyLeft(Matrix3 mat) {
		int i, j, k;
		float[] ans;
		float sum;
		float[][] multp;

		multp = new float[SIZE][SIZE];

		for ( i = 0; i < SIZE; i++) {
			for (j = 0; j < SIZE; j++) {
				multp[i][j] = mat.get(i, j);
			}
		}
		
		ans = new float[SIZE*SIZE];

		for (i = 0; i < SIZE; i++) {
			for (j=0; j < SIZE; j++) {
				sum = 0.0F;
				for (k = 0; k < SIZE; k++) {
					sum = sum + multp[i][k]*matrix[k][j];
				}
				ans[(i*SIZE)+j] = sum;
			}
		}

		return new Matrix3(ans);

	}		

	public Matrix3 multiplyRight(Matrix3 mat) {
		int i, j, k;
                float[] ans;
                float sum;
                float[][] multp;

                multp = new float[SIZE][SIZE];

                for ( i = 0; i < SIZE; i++) {
                        for (j = 0; j < SIZE; j++) {
                                multp[i][j] = mat.get(i, j);
                        }
                }

                ans = new float[SIZE*SIZE];

                for (i = 0; i < SIZE; i++) {
                        for (j=0; j < SIZE; j++) {
                                sum = 0.0F;
                                for (k = 0; k < SIZE; k++) {
                                        sum = sum + matrix[i][k]*multp[k][j];
                                }
                                ans[i*SIZE+j] = sum;
                        }
                }

                return new Matrix3(ans);
	}

	public float[] multiplyRowVector(float[] vec) {
		int i, j, k;
		float[] ans;
		float sum;
		
		ans = new float[SIZE];
		
		for (i = 0; i < SIZE; i++) {
			sum = 0.0F;
			for (k = 0; k < SIZE; k++) {
				sum = sum + vec[k] * matrix[i][k];
			}
			ans[i] = sum;
		}

		return ans;
	}

	public float[] multiplyColVector(float[] vec) {
                int i, j, k;
                float[] ans;
                float sum;

                ans = new float[SIZE*SIZE];

                for (i = 0; i < SIZE; i++) {
                	sum = 0.0F;
              		for (k = 0; k < SIZE; k++) {
                      		sum = sum + matrix[k][i]*vec[k];
                        }
              		ans[i] = sum;
                }
		
		return ans;
	}
}
