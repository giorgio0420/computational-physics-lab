#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(){

double a,b,c,x0,y0,z0,dt,T;
a=0.1;
b=0.1;
c=1;
x0=0;
y0=0;
z0=0;
dt=0.01;
T=2000;



double x,y,z,  xs,ys,zs, t;
  
 t=0.;
  int n= T/dt;
  
 
 printf("#a=%f,b=%f,c=%f, x0=%f,y0=%f,z0=%f, dt=%f, T=%f\n", a,b,c, x0,y0,z0, dt,T);
  
  x=x0;
  y=y0;
  z=z0;

  double kx1,kx2,kx3,kx4,ky1,ky2,ky3,ky4,kz1,kz2,kz3,kz4;
  
  //variabili per inteerpolazione lineare tra le y e x per gli zeri
  double mx,my,qx,qy,t0,xbif;



for(int i1=0; i1<73; i1++){
  
  for(int i=0; i<n; i++){

    kx1= (-y -z)*dt;
    ky1= (x +a*y)*dt;
    kz1= (b +x*z -c*z)*dt;
    
    kx2= ((-y-ky1/2) + (-z-kz1/2))*dt;
    ky2= ((x+kx1/2) + a*(y+ky1/2))*dt;
    kz2= (b + (x+kx1/2)*(z+kz1/2) - c*(z+kz1/2))*dt;
    
    kx3= ((-y-ky2/2) + (-z-kz2/2))*dt;
    ky3= ((x+kx2/2) + a*(y+ky2/2))*dt;
    kz3= (b + (x+kx2/2)*(z+kz2/2) - c*(z+kz2/2))*dt;    
    
    kx4= ((-y-ky3/2) + (-z-kz3/2))*dt;
    ky4= ((x+kx3/2) + a*(y+ky3/2))*dt;
    kz4= (b + (x+kx3/2)*(z+kz3/2) - c*(z+kz3/2))*dt;    
 
 
    xs= x + (kx1+kx2*2+2*kx3+kx4)/6;
    ys= y + (ky1+ky2*2+2*ky3+ky4)/6;
    zs= z + (kz1+kz2*2+2*kz3+kz4)/6;
    
          t+=dt;

    if(((ys>0)&(y<0))||((ys<0)&(y>0))){
    
    if(x>0){
    
    my=(ys-y)/dt;
    qy=y-my*t;
    
    //tempo in cui y=0 con interpolazione lineare
    t0= -qy/my;
    
    mx=(xs-x)/dt;
    qx=x-mx*t; 
    
    xbif= mx*t0 + qx;
    
    printf("%f  %f\n", c, xbif);
   
	    }
	}
       
        x= xs;
        y= ys;
        z= zs;
  }

c+= 0.125;

}

}
