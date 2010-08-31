package org.web3d.x3d.sai;

public class Matrix4 {
        public float[][] matrix;
        public static int SIZE = 4;
        public Matrix4() {
                int i;
                int j;
                matrix = new float[SIZE][SIZE];
                for (i = 0; i < SIZE; i++) {
                        for (j=0; j<SIZE; j++) {
                                matrix[i][j] = 0;
                        }
                }
        }

        public Matrix4(float[][] init) {
                int i;
                int j;

                matrix = new float[SIZE][SIZE];

                for (i = 0; i < SIZE; i++) {
                        for (j = 0; j < SIZE; j++) {
                                matrix[i][j] = init[i][j];
                        }
                }
	}

        public Matrix4(float[] init) {
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

	public void setTransform(SFVec3f translation, SFRotation rotation, SFVec3f scale, SFRotation scaleOrientation, SFVec3f centre) {
		float[][] rot;
		float[][] finalscale;
		float[][] srot;
		float[][] nsrot;
		float[][] strans;
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
		nsrot = new float[SIZE][SIZE];
		trans = new float[SIZE][SIZE];
		strans = new float[SIZE][SIZE];
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
					nsrot[i][j] = 1.0F;
					trans[i][j] = 1.0F;
					strans[i][j] = 1.0F;
					nstrans[i][j] = 1.0F;
					sc[i][j] = 1.0F;
					finalscale[i][j] = 1.0F;
				} else {
					trans[i][j] = 0.0F;
					strans[i][j] = 0.0F;
					nstrans[i][j] = 0.0F;
					sc[i][j] = 0.0F;
					rot[i][j] = 0.0F;
					srot[i][j] = 0.0F;
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

			trans[3][0] = value[0];
			trans[3][1] = value[1];
			trans[3][2] = value[2];
		}

		if (scale != null) {
			try {
				scale.getValue(value);
			} catch (Exception e) {
				System.out.println(e);
			}

			sc[0][0] = value[0];
			sc[1][1] = value[1];
			sc[2][2] = value[2];

			if (centre != null) {
                       		try {
                        	        centre.getValue(value);
                        	} catch (Exception e) {
                        	        System.out.println(e);
                        	}

                        	strans[3][0] = value[0];
                        	strans[3][1] = value[1];
                        	strans[3][2] = value[2];

			}

			if (scaleOrientation!=null) {
                        	try {
                        	        scaleOrientation.getValue(axisr);
                        	} catch (Exception e) {
                        	        System.out.println(e);
                        	}

                        	length = (float) (Math.sqrt(axisr[0]*axisr[0] + axisr[1]*axisr[1] + axisr[2]*axisr[2]));

                        	x = axisr[0]/length;
                        	y = axisr[1]/length;
                        	z = axisr[2]/length;


                        	c = (float)(Math.cos(axisr[3]));
                        	s = (float)(Math.sin(axisr[3]));
                        	t = 1-c;

                        	srot[0][0] = t*x*x + c;
                        	srot[0][1] = t*x*y - z*s;
                        	srot[0][2] = t*x*z + y*s;
                        	srot[1][0] = t*x*y + z*s;
                        	srot[1][1] = t*y*y + c;
                        	srot[1][2] = t*y*z - x*s;
                        	srot[2][0] = t*x*z - y*s;
                        	srot[2][1] = t*y*z + x*s;
                        	srot[2][2] = t*z*z + c;
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

			length = (float)(Math.sqrt(axisr[0]*axisr[0] + axisr[1]*axisr[1] + axisr[2]*axisr[2]));

			x = axisr[0]/length;
			y = axisr[1]/length;
			z = axisr[2]/length;

		
			c = (float)(Math.cos(axisr[3]));
			s = (float)(Math.sin(axisr[3]));
			t = 1-c;

			rot[0][0] = t*x*x + c;
			rot[0][1] = t*x*y - z*s;
			rot[0][2] = t*x*z + y*s;
			rot[1][0] = t*x*y + z*s;
			rot[1][1] = t*y*y + c;
			rot[1][2] = t*y*z - x*s;
			rot[2][0] = t*x*z - y*s;
			rot[2][1] = t*y*z + x*s;
			rot[2][2] = t*z*z + c;
		}

		matrix = multiply(matrix, trans);
		matrix = multiply(matrix, strans);
		matrix = multiply(matrix, rot);
		matrix = multiply(matrix, srot);
		matrix = multiply(matrix, sc);
		matrix = multiply(matrix, nsrot);
		matrix = multiply(matrix, nstrans);
	}

	public void getTransform(SFVec3f translation, SFRotation rotation, SFVec3f scale) {
		float[] t;
		float[] s;
		float[] r;

		r = new float[SIZE];
		s = new float[SIZE];
		t = new float[SIZE];

		t[0] = matrix[0][3];
		t[1] = matrix[1][3];
		t[2] = matrix[2][3];

		translation.setValue(t);

		// Note that this only works if the scale transform was applied first - since that is how transforms are applied in VRML, and this is a much 
		// more efficient way of doing things we make that assumption

		s[0]  = (float) (Math.sqrt(matrix[0][0]*matrix[0][0] + matrix[1][0]*matrix[1][0] + matrix[2][0]*matrix[2][0] + matrix[3][0]*matrix[3][0]));
		s[1]  = (float) (Math.sqrt(matrix[0][1]*matrix[0][1] + matrix[1][1]*matrix[1][1] + matrix[2][1]*matrix[2][1] + matrix[3][1]*matrix[3][1]));
		s[2]  = (float) (Math.sqrt(matrix[0][2]*matrix[0][2] + matrix[1][2]*matrix[1][2] + matrix[2][2]*matrix[2][2] + matrix[3][2]*matrix[3][2]));

		scale.setValue(s);

		r[3] = (float) (Math.acos((matrix[0][0] + matrix[1][1] + matrix[2][2] - 1)/2.0F));
		r[0] = (float)((matrix[2][1]-matrix[1][2])/(Math.sqrt((matrix[2][1]-matrix[1][2])*(matrix[2][1]-matrix[1][2]) + (matrix[0][2]-matrix[2][0])*(matrix[0][2]*matrix[2][0]) + (matrix[1][0]-matrix[0][1])*(matrix[1][0]-matrix[0][1]))));
		r[1] = (float)((matrix[0][2]-matrix[2][0])/(Math.sqrt((matrix[2][1]-matrix[1][2])*(matrix[2][1]-matrix[1][2]) + (matrix[0][2]-matrix[2][0])*(matrix[0][2]*matrix[2][0]) + (matrix[1][0]-matrix[0][1])*(matrix[1][0]-matrix[0][1]))));
		r[2] = (float)((matrix[1][0]-matrix[0][1])/(Math.sqrt((matrix[2][1]-matrix[1][2])*(matrix[2][1]-matrix[1][2]) + (matrix[0][2]-matrix[2][0])*(matrix[0][2]*matrix[2][0]) + (matrix[1][0]-matrix[0][1])*(matrix[1][0]-matrix[0][1]))));

		rotation.setValue(r);
	}

	public Matrix4 inverse() {
		float[] transp;
		float[] dst;
		float[] tmp;
		float[] mymatrix;
		int i, j, count;
		float A;

		transp = new float[16];
		dst = new float[16];
		tmp = new float[12];	
		mymatrix = new float[16];

		count = 0;

		A = matrix[0][0]*matrix[1][1]*matrix[2][2]*matrix[3][3] + matrix[0][0]*matrix[1][2]*matrix[2][3]*matrix[3][1] + matrix[0][0]*matrix[1][3]*matrix[2][1]*matrix[3][2];
		A += matrix[0][1]*matrix[1][0]*matrix[2][3]*matrix[3][2] + matrix[0][1]*matrix[1][2]*matrix[2][0]*matrix[3][3] + matrix[0][1]*matrix[1][3]*matrix[2][2]*matrix[3][0];
		A += matrix[0][2]*matrix[1][0]*matrix[2][1]*matrix[3][3] + matrix[0][2]*matrix[1][1]*matrix[2][3]*matrix[3][0] + matrix[0][2]*matrix[1][3]*matrix[2][0]*matrix[3][1];
		A += matrix[0][3]*matrix[1][0]*matrix[2][2]*matrix[3][1] + matrix[0][3]*matrix[1][1]*matrix[2][0]*matrix[3][2] + matrix[0][3]*matrix[1][2]*matrix[2][1]*matrix[3][0];
		A = A - matrix[0][0]*matrix[1][1]*matrix[2][3]*matrix[3][2] - matrix[0][0]*matrix[1][2]*matrix[2][1]*matrix[3][3] - matrix[0][0]*matrix[1][3]*matrix[2][2]*matrix[3][1];
		A = A - matrix[0][1]*matrix[1][0]*matrix[2][2]*matrix[3][3] - matrix[0][1]*matrix[1][2]*matrix[2][3]*matrix[3][0] - matrix[0][1]*matrix[1][3]*matrix[2][0]*matrix[3][2];
		A = A - matrix[0][2]*matrix[1][0]*matrix[2][3]*matrix[3][1] - matrix[0][2]*matrix[1][1]*matrix[2][0]*matrix[3][3] - matrix[0][2]*matrix[1][3]*matrix[2][1]*matrix[3][0];
		A = A - matrix[0][3]*matrix[1][0]*matrix[2][1]*matrix[3][2] - matrix[0][3]*matrix[1][1]*matrix[2][2]*matrix[3][0] - matrix[0][3]*matrix[1][2]*matrix[2][0]*matrix[3][1];

		if (A == 0) {
			return null;
		} else {
			A = 1.0F/A;
		}

		dst[0] = matrix[1][1]*matrix[2][2]*matrix[3][3] + matrix[1][2]*matrix[2][3]*matrix[3][1] + matrix[1][3]*matrix[2][1]*matrix[3][2];
		dst[0] = dst[0] - matrix[1][1]*matrix[2][3]*matrix[3][2] - matrix[1][2]*matrix[2][1]*matrix[3][3] - matrix[1][3]*matrix[2][2]*matrix[3][1];
		dst[1] = matrix[0][1]*matrix[2][3]*matrix[3][2] + matrix[0][2]*matrix[2][1]*matrix[3][3] + matrix[0][3]*matrix[2][2]*matrix[3][1];
		dst[1] = dst[1] - matrix[0][1]*matrix[2][2]*matrix[3][3] - matrix[0][2]*matrix[2][3]*matrix[3][1] - matrix[0][3]*matrix[2][1]*matrix[3][2];
		dst[2] = matrix[0][1]*matrix[1][2]*matrix[3][3] + matrix[0][2]*matrix[1][3]*matrix[3][1] + matrix[0][3]*matrix[1][1]*matrix[3][2];
		dst[2] = dst[2] - matrix[0][1]*matrix[1][3]*matrix[3][2] - matrix[0][2]*matrix[1][1]*matrix[3][3] - matrix[0][3]*matrix[1][2]*matrix[3][1];
		dst[3] = matrix[0][1]*matrix[1][3]*matrix[2][2] + matrix[0][2]*matrix[1][1]*matrix[2][3] + matrix[0][3]*matrix[1][2]*matrix[2][1];
		dst[3] = dst[3] - matrix[0][1]*matrix[1][2]*matrix[2][3] - matrix[0][2]*matrix[1][3]*matrix[2][1] - matrix[0][3]*matrix[1][1]*matrix[2][2];
		dst[4] = matrix[1][0]*matrix[2][3]*matrix[3][2] + matrix[1][2]*matrix[2][0]*matrix[3][3] + matrix[1][3]*matrix[2][2]*matrix[3][0];
		dst[4] = dst[4] - matrix[1][0]*matrix[2][2]*matrix[3][3] - matrix[1][2]*matrix[2][3]*matrix[3][0] - matrix[1][3]*matrix[2][0]*matrix[3][2];
		dst[5] = matrix[0][0]*matrix[2][2]*matrix[3][3] + matrix[0][2]*matrix[2][3]*matrix[3][0] + matrix[0][3]*matrix[2][0]*matrix[3][2];
		dst[5] = dst[5] - matrix[0][0]*matrix[2][3]*matrix[3][2] - matrix[0][2]*matrix[2][0]*matrix[3][3] - matrix[0][3]*matrix[2][2]*matrix[3][0];
		dst[6] = matrix[0][0]*matrix[1][3]*matrix[3][2] + matrix[0][2]*matrix[1][0]*matrix[3][3] + matrix[0][3]*matrix[1][2]*matrix[3][0];
		dst[6] = dst[6] - matrix[0][0]*matrix[1][2]*matrix[3][3] - matrix[0][2]*matrix[1][3]*matrix[3][0] - matrix[0][3]*matrix[1][0]*matrix[3][2];
		dst[7] = matrix[0][0]*matrix[1][2]*matrix[2][3] + matrix[0][2]*matrix[1][3]*matrix[2][0] + matrix[0][3]*matrix[1][0]*matrix[2][2];
		dst[7] = dst[7] - matrix[0][0]*matrix[1][3]*matrix[2][2] - matrix[0][2]*matrix[1][0]*matrix[2][3] - matrix[0][3]*matrix[1][2]*matrix[2][0];
		dst[8] = matrix[1][0]*matrix[2][1]*matrix[3][3] + matrix[1][1]*matrix[2][3]*matrix[3][0] + matrix[1][3]*matrix[2][0]*matrix[3][1];
		dst[8] = dst[8] - matrix[1][0]*matrix[2][3]*matrix[3][1] - matrix[1][1]*matrix[2][0]*matrix[3][3] - matrix[1][3]*matrix[2][1]*matrix[3][0];
		dst[9] = matrix[0][0]*matrix[2][3]*matrix[3][1] + matrix[0][1]*matrix[2][0]*matrix[3][3] + matrix[0][3]*matrix[2][1]*matrix[3][0];
		dst[9] = dst[9] - matrix[0][0]*matrix[2][1]*matrix[3][3] - matrix[0][1]*matrix[2][3]*matrix[3][0] - matrix[0][3]*matrix[2][0]*matrix[3][1];
		dst[10] = matrix[0][0]*matrix[1][1]*matrix[3][3] + matrix[0][1]*matrix[1][3]*matrix[3][0] + matrix[0][3]*matrix[1][0]*matrix[3][1];
		dst[10] = dst[10] - matrix[0][0]*matrix[1][3]*matrix[3][1] - matrix[0][1]*matrix[1][0]*matrix[3][3] - matrix[0][3]*matrix[1][1]*matrix[3][0];
		dst[11] = matrix[0][0]*matrix[1][3]*matrix[2][1] + matrix[0][1]*matrix[1][0]*matrix[2][3] + matrix[0][3]*matrix[1][1]*matrix[2][0];
		dst[11] = dst[11] - matrix[0][0]*matrix[1][1]*matrix[2][3] - matrix[0][1]*matrix[1][3]*matrix[2][0] - matrix[0][3]*matrix[1][0]*matrix[2][1];
		dst[12] = matrix[1][0]*matrix[2][2]*matrix[3][1] + matrix[1][1]*matrix[2][0]*matrix[3][2] + matrix[1][2]*matrix[2][1]*matrix[3][0];
		dst[12] = dst[12] - matrix[1][0]*matrix[2][1]*matrix[3][2] - matrix[1][1]*matrix[2][2]*matrix[3][0] - matrix[1][2]*matrix[2][0]*matrix[3][1];
		dst[13] = matrix[0][0]*matrix[2][1]*matrix[3][2] + matrix[0][1]*matrix[2][2]*matrix[3][0] + matrix[0][2]*matrix[2][0]*matrix[3][1];
		dst[13] = dst[13] - matrix[0][0]*matrix[2][2]*matrix[3][1] - matrix[0][1]*matrix[2][0]*matrix[3][2] - matrix[0][2]*matrix[2][1]*matrix[3][0];
		dst[14] = matrix[0][0]*matrix[1][2]*matrix[3][1] + matrix[0][1]*matrix[1][0]*matrix[3][2] + matrix[0][2]*matrix[1][1]*matrix[3][0];
		dst[14] = dst[14] - matrix[0][0]*matrix[1][1]*matrix[3][2] - matrix[0][1]*matrix[1][2]*matrix[3][0] - matrix[0][2]*matrix[1][0]*matrix[3][1];
		dst[15] = matrix[0][0]*matrix[1][1]*matrix[2][2] + matrix[0][1]*matrix[1][2]*matrix[2][0] + matrix[0][2]*matrix[1][0]*matrix[2][1];
		dst[15] = dst[15] - matrix[0][0]*matrix[1][2]*matrix[2][1] - matrix[0][1]*matrix[1][0]*matrix[2][2] - matrix[0][2]*matrix[1][1]*matrix[2][0];


		for (j = 0; j < 16; j++)
			dst[j] *= A;
		
		return new Matrix4(dst);
	}
 
	public Matrix4 transpose() {
		float[] transp;
		int i, j;

		transp = new float[16];

		for (i=0; i < SIZE; i++) {
			for (j=0; j < SIZE; j++) {
				transp[j*SIZE + i] = matrix[i][j];
			}
		}
		
		return new Matrix4(transp);
	}

	public Matrix4 multiplyLeft(Matrix4 mat) {
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
        
                return new Matrix4(ans);
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
        

	public Matrix4 multiplyRight(Matrix4 mat) {
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

                return new Matrix4(ans);
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
