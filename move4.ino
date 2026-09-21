void moveeee(int aaaa, int bbbb, int cccc,float x, float y, float z , float* thetaH4, float *thetaT4, float* thetaK4){
 ik4( x,  y ,  z, thetaH4,  thetaT4,  thetaK4);

*thetaH4=*thetaH4 + 60 ;
*thetaT4=85 -*thetaT4;
*thetaK4=  *thetaK4+0;
write( aaaa, *thetaH4);
write( bbbb, *thetaT4);
write( cccc,*thetaK4 );}