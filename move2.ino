void movee(int aa, int bb, int cc,float xx, float yy, float zz , float* thetaH2, float *thetaT2, float* thetaK2){
 ik2( xx,  yy ,  zz, thetaH2,  thetaT2,  thetaK2);

*thetaH2= -*thetaH2 + 95 ;
*thetaT2=60-*thetaT2;
*thetaK2= *thetaK2-10;
write( aa, *thetaH2);
write( bb, *thetaT2);
write( cc,*thetaK2 );}