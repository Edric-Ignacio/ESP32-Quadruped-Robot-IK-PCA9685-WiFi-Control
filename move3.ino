void moveee(int aaa, int bbb, int ccc,float xx, float yy, float zz , float* thetaH3, float *thetaT3, float* thetaK3){
 ik3( xx,  yy ,  zz, thetaH3,  thetaT3,  thetaK3);

*thetaH3= -*thetaH3 + 65 ;
*thetaT3=90+*thetaT3;
*thetaK3=165- *thetaK3;
write( aaa, *thetaH3);
write( bbb, *thetaT3);
write( ccc,*thetaK3 );}