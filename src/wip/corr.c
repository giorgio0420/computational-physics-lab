#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define l 10000

int main(){

double t0,t1;
t0=1989;
t1=2001;


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
  double x1[l], y1[l], z1[l];  

  //variabili per periodo
  double t1x, t0x, t1y, t0y, t0z, t1z, periodox, periodoy, periodoz;
  double xm,ym,zm;

  xm=x0;
  zm=z0;
  ym=y0;
  x=x0;
  y=y0;
  z=z0;

   int g=0, qx=0,qy=0,qz=0;

  for(int i=0; i<n; i++){
    

if(i>189999){
   g+=1;
    x1[g]= xs;
    y1[g]= ys;
    z1[g]= zs;
}

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
       
       if((t0<t)&(t<t1)){
       if((x<xs) & (x>xm)){
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
       if((y<ys) & (y>ym)){
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
        if((z<zs) & (z>zm)){
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
 printf("#period averaged over the three variables: %f\n", mperiodi);

//passi in un periodo
int k=0;
double corxy[l], corxz[l], somxy,somxz;
 

 printf("#k, somxy/mperiodi, somyz/mperiodi, somxz/mperiodi\n");
for(int i1=0; i1<50; i1++){

somxy=0;
somxz=0;

for(int i2=0; i2<l; i2++){
//correlazione tra x_y,y_z,x_z
corxy[i2]=x1[i2]*y1[k+i2];
corxz[i2]=x1[i2]*z1[k+i2];
}

for(int i3=0; i3<l; i3++){
//correlazione tra x_y,x_z
somxy+=corxy[i3];
somxz+=corxz[i3];
printf("%d  %f %f\n", k, somxy/mperiodi/1000000/2.33, somxz/mperiodi/600000);
}

k+=12;
}


}
