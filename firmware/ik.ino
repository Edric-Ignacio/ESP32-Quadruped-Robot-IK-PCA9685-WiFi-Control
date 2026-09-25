float l = 55;
float l2 = 57 ;
 void ik(float x, float y , float z, float* thetaH, float *thetaT, float* thetaK) {
  float R = sqrt(x*x+z*z);
  float gama =  atan2(x, z);
  float knee = acos( (l*l + l2*l2 - R*R)/(2*l*l2));
  *thetaK = PI-knee;
  float thigh = acos( (l*l - l2*l2 + R*R)/(2*l*R));
  *thetaT= gama - thigh;
   *thetaH = -atan2 (y, z);

   *thetaH= *thetaH* 180/PI;
   *thetaT= *thetaT*180/PI;
   *thetaK= *thetaK *180/PI;
 }
