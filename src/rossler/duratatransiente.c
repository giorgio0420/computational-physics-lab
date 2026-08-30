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
  

  double kx1,kx2,ky1,ky2,kz1,kz2;
  
  //variabili per periodo
  double tx, ty, tz, max1x,max2x, max1y,max2y, max1z,max2z;
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
     
     if(t>100){   
       
      if((x>xs) & (x>xm)){
	if(qx==1){
	qx= 2;
	max2x=x;
	if((max2x-max1x)<0.0005){
	tx=t;
	}else{
	qx=1;
	max1x=max2x;
	}
	}
	if(qx==0){
	qx= 1;
	max1x=x;
	}
	}
	
     if((y>ys) & (y>ym)){
	if(qy==1){
	qy= 2;
	max2y=y;
	if((max2y-max1y)<0.0005){
	ty=t;
	}else{
	qy=1;
	max1y=max2y;
	}
	}
	if(qy==0){
	qy= 1;
	max1y=y;
	}
	}
	
     if((z>zs) & (z>zm)){
	if(qz==1){
	qz= 2;
	max2z=z;
	if((max2z-max1z)<0.0005){
	tz=t;
	}else{
	qz=1;
	max1z=max2z;
	}
	}
	if(qz==0){
	qz= 1;
	max1z=z;
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
  
       printf("transient duration per variable:\ntx=%f,\nty=%f,\ntz=%f.\n",tx,ty,tz);
       
  //calcolo massimo tra i tre tempi trovati nei quali finisce il transiente
  if((tx>ty)&(tx>tz)){
   printf("transient duration of the system: %f\n", tx); 
   }
  if((ty>tz)&(ty>tx)){
   printf("transient duration of the system: %f\n", ty); 
   }
  if((tz>ty)&(tz>tx)){
   printf("transient duration of the system: %f\n", tz); 
   } 
  
  
  }
  
  
