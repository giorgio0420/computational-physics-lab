#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(){

double a,b,c,x0,y0,z0,dt,T;
a=0.1;
b=0.1;
x0=0;
y0=0;
z0=0;
dt=0.01;
T=1500;

double x,y,z,  xs,ys,zs, t;
  
 t=0.;
 
  int n= T/dt;
   
  double kx1,kx2,kx3,kx4,ky1,ky2,ky3,ky4,kz1,kz2,kz3,kz4;
  
  //variabili per periodo
  double tx, ty, tz, max1x,max2x, max1y,max2y, max1z,max2z;
  double xm,ym,zm;

double duratatransiente;



c=0.399;

for(int e=0; e<601; e++){

  xm=x0;
  zm=z0;
  ym=y0;
  x=x0;
  y=y0;
  z=z0;

c+=0.001;

t=0;

   int qx=0,qy=0,qz=0;

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
      
  //calcolo massimo tra i tre tempi trovati nei quali finisce il transiente
  if((tx>ty)&(tx>tz)){
   duratatransiente=tx;}
  if((ty>tz)&(ty>tx)){
   duratatransiente=ty;}
  if((tz>ty)&(tz>tx)){
   duratatransiente=tz;} 



//calcolo lunghezza integrale di linea della derivata del modulo r per trovare la lunghezza del trasiente
        double dr,drs;
        double dx,dy,dz, kr1,kr2,kr3,kr4;

  x=x0;
  y=y0;
  z=z0;
 


 
 n= (int)duratatransiente/dt;
 double lunghezzacurva=0;
 t=0;
 
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
 
        dx=-y-z;
        dy=x+a*y;
        dz=b+(x-c)*z;

        //modulo, derivata modulo
        dr= sqrt(pow(dx,2)+pow(dy,2)+pow(dz,2));

   kr1= sqrt(pow((-y+ky1/2-z+kz1/2),2)+pow((x+kx1/2+0.1*(y+ky1/2)),2)+pow((0.1+(x+kx1/2-c)*(z+kz1/2)),2))*dt;
   kr2= sqrt(pow((-y+ky2/2-z+kz2/2),2)+pow((x+kx2/2+0.1*(y+ky2/2)),2)+pow((0.1+(x+kx2/2-c)*(z+kz2/2)),2))*dt;
   kr3= sqrt(pow((-y+ky3/2-z+kz3/2),2)+pow((x+kx3/2+0.1*(y+ky3/2)),2)+pow((0.1+(x+kx3/2-c)*(z+kz3/2)),2))*dt;
   kr4= sqrt(pow((-y+ky4/2-z+kz4/2),2)+pow((x+kx4/2+0.1*(y+ky4/2)),2)+pow((0.1+(x+kx4/2-c)*(z+kz4/2)),2))*dt;
    
     drs= dr + (kr1+kr2*2+2*kr3+kr4)/6;
  
  lunghezzacurva+= drs;
        
        t+=dt;
         
    x= xs;
    y= ys;
    z= zs;
    dr=drs;
 }
      
  printf("%f %f %f\n", c, lunghezzacurva, duratatransiente);
  
}

}
  
  
