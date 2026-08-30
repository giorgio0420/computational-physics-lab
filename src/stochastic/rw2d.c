#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define D 2
#define _USE_MATH_DEFINES

int main(int argc, char *argv[argc]){

int Tmax, traiettorie, x, y;
unsigned long long seme = 12345;   /* default seed, override with argv[3] */

if(argc < 3 || argc > 4){
fprintf(stderr, "usage: %s <n_trajectories> <t_max> [seed]\n", argv[0]);
exit(EXIT_FAILURE);
}

traiettorie= atoi(argv[1]);
Tmax= atoi(argv[2]);
if(argc == 4) seme = strtoull(argv[3], NULL, 10);
  
  int h;
 
    printf("# t    x      y       P(x)     P(y) \n");

double px,py, pxy;
  
 for(h=0; h<traiettorie; h++){

  int a=1;
  
      for(int t=0; t<(Tmax+1); t++){
    
      if(t==0){
       x=0;
       y=0;
      }	
      else{
      seme = (16807*seme)%2147483647;
      double r = (double)seme/2147483647.0;

      if(r<=0.25){
	x += 1;
      }
      if((r>0.25)&(r<=0.5)){
	x -= 1;
      }
      if((r>0.5)&(r<=0.75)){
	y += 1;
      }
      if(r>0.75){
	y -= 1;
      }


if(t==(pow(10,5))){
//if(t==Tmax){
   px = sqrt(1/M_PI/t)*exp(-(double)x*x/t);
   py = sqrt(1/M_PI/t)*exp(-(double)y*y/t);

   pxy= 1/M_PI/t*exp(-(double)(x*x+y*y)/t);

    printf("%.7d  %.4d  %.4d %.10f %.10f %.10f\n\n", t, x, y, px, py, pxy);

a+=1;	}
      	}
      						
   	}	

printf("\n");

	 }

   }
