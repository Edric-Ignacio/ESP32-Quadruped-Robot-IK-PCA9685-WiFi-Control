void move(int a, int b, int c,float x, float y, float z , float* thetaH, float *thetaT, float* thetaK){
 ik( x,  y ,  z, thetaH,  thetaT,  thetaK);

*thetaH= *thetaH +105 ;
*thetaT=90 +*thetaT;
*thetaK= 170- *thetaK;
write( a, *thetaH);
write( b, *thetaT);
write( c,*thetaK );

} 