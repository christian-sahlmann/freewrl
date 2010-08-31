package org.web3d.x3d.sai;

public interface Matrix {
        public void setTransform(SFVec3f translation, SFVec3f rotation, SFVec2f scale, SFVec3f scaleOrientation, SFVec2f center);
        public void getTransform (SFVec2f translation, SFVec3f rotation, SFVec2f scale);
        public void inverse(float[][] matrix);
        public void transpose(float[][] matrix);
        public void multiplyLeft(float[][] matrix, float[][] mult, int size);
        public void multiplyRight(float[][] matrix, float[][] mult, int size);
        public void multiplyRowVector(float[][] matrix, float[] vec, int size);
        public void multiplyColVector(float [][] matrix, float[] vec, int size);
}
