
 void ik4(float x, float y , float z, float* thetaH4, float *thetaT4, float* thetaK4) {
  float r4 = sqrt(x*x+z*z);
  float gama4 =  atan2(x, z);
  float knee4 = acos( (l*l + l2*l2 - r4*r4)/(2*l*l2));
  *thetaK4 = PI-knee4;
  float thigh = acos( (l*l - l2*l2 + r4*r4)/(2*l*r4));
  *thetaT4= gama4 - thigh;
   *thetaH4 = -atan2 (y, z);

   *thetaH4= *thetaH4* 180/PI;
   *thetaT4= *thetaT4*180/PI;
   *thetaK4= *thetaK4 *180/PI;
 }