#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[argc]){

double a,b,c,x0,y0,z0,dt,T;

if(argc != 9){
fprintf(stderr, "usage: %s a b c x0 y0 z0 dt T\n\n  a, b, c    Rossler parameters\n  x0, y0, z0 initial conditions\n  dt         integration step\n  T          total integration time\n", argv[0]);
exit(EXIT_FAILURE);
}

a= atof(argv[1]);
b= atof(argv[2]);
c= atof(argv[3]);
x0= atof(argv[4]);
y0= atof(argv[5]);
z0= atof(argv[6]);
dt= atof(argv[7]);
T= atof(argv[8]);


double x,y,z,  xs,ys,zs, t;
  
 t=0.;
  int n= T/dt;
  
 
 printf("#a=%f,b=%f,c=%f, x0=%f,y0=%f,z0=%f, dt=%f, T=%f\n", a,b,c, x0,y0,z0, dt,T);
  
  x=x0;
  y=y0;
  z=z0;
 
  double kx1,kx2,ky1,ky2,kz1,kz2;
  
  for(int i=0; i<n; i++){

    kx1= (-y -z)*dt;
    ky1= (x +a*y)*dt;
    kz1= (b +x*z -c*z)*dt;
    
    kx2= ((-y-ky1/2) + (-z-kz1/2))*dt;
    ky2= ((x+kx1/2) + a*(y+ky1/2))*dt;
    kz2= (b + (x+kx1/2)*(z+kz1/2) - c*(z+kz1/2))*dt;
    
    xs= x + (kx1+kx2)/2;
    ys= y + (ky1+ky2)/2;
    zs= z + (kz1+kz2)/2;
    
    t+=dt; 

        printf("%f  %.10f  %.10f  %.10f\n", t, x, y,z);

    x= xs;
    y= ys;
    z= zs;
  }
  

}

