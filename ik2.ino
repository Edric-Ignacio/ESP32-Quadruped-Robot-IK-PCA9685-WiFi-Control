
 void ik2(float xx, float yy , float zz, float* thetaH2, float *thetaT2, float* thetaK2) {
  float R2 = sqrt(xx*xx+zz*zz);
  float gama2 =  atan2(xx, zz);
  float knee2 = acos( (l*l + l2*l2 - R2*R2)/(2*l*l2));
  *thetaK2 = PI-knee2;
  float thigh2 = acos( (l*l - l2*l2 + R2*R2)/(2*l*R2));
  *thetaT2= gama2 - thigh2;
   *thetaH2 = -atan2 (yy, zz);

   *thetaH2= *thetaH2* 180/PI;
   *thetaT2= *thetaT2*180/PI;
   *thetaK2= *thetaK2 *180/PI;
 }