 void ik3(float xx, float yy , float zz, float* thetaH3, float *thetaT3, float* thetaK3) {
  float r3 = sqrt(xx*xx+zz*zz);
  float gama3 =  atan2(xx, zz);
  float knee3 = acos( (l*l + l2*l2 - r3*r3)/(2*l*l2));
  *thetaK3 = PI-knee3;
  float thigh = acos( (l*l - l2*l2 + r3*r3)/(2*l*r3));
  *thetaT3= gama3 - thigh;
   *thetaH3 = -atan2 (yy, zz);

   *thetaH3= *thetaH3* 180/PI;
   *thetaT3= *thetaT3*180/PI;
   *thetaK3= *thetaK3 *180/PI;
 }
