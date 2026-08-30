#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(){

double t0,t1;
t0=1889;
t1=2001;


double a,b,c,x0,y0,z0,dt,T;
a=0.1;
b=0.1;
c=1;
x0=0;
y0=0;
z0=0;
dt=0.0001;
T=2000;

double x,y,z,  xs,ys,zs, t;
  
 t=0.;
 
  int n= T/dt;
   
 printf("#a=%.3f,  b=%.3f,  c=%.3f,    x0=%.3f, y0=%.3f, z0=%.3f,   dt=%.3f,  T=%.3f\n", a,b,c, x0,y0,z0, dt,T);
  

  double kx1,kx2,ky1,ky2,kz1,kz2; 

  //variabili per periodo
  double t1x, t0x, t1y, t0y, t0z, t1z, periodox=0, periodoy=0, periodoz=0;
  double xm,ym,zm;

  xm=x0;
  zm=z0;
  ym=y0;
  x=x0;
  y=y0;
  z=z0;

   int qx=0,qy=0,qz=0;

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
       
       if((t>t0)&(t<t1)){
       if((x>xs) & (x>xm)){
	if(qx==1){
	qx= 2;
	t1x=t;
	periodox = t1x-t0x;
	}
	if(qx==0){
	qx= 1;
	t0x=t;
	}
	}
       if((y>ys) & (y>ym)){
       	if(qy==1){
	qy= 2;
	t1y=t;
	periodoy = t1y-t0y;
	}
	if(qy==0){
	qy= 1;
	t0y=t;
	}
	}
        if((z>zs) & (z>zm)){
       	if(qz==1){
	qz= 2;
	t1z=t;
	periodoz = t1z-t0z;
	}
	if(qz==0){
	qz= 1;
	t0z=t;
	}
	}
       
       
     }  
       
    xm= x;
    ym= y;
    zm= z;
    x= xs;
    y= ys;
    z= zs;
  }
  

  double mperiodi= (periodox+periodoy+periodoz)/3;
 printf("period from successive maxima of x: %.5f-%.5f = %.5f\n", t1x,t0x,periodox);
 printf("period from successive maxima of y: %.5f-%.5f = %.5f\n", t1y,t0y,periodoz);
 printf("period from successive maxima of z: %.5f-%.5f = %.5f\n", t1z,t0z,periodoy);
 printf("#period averaged over the three variables: %.5f\n", mperiodi);
 
 
 }
 
