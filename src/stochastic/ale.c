#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char *argv[argc]){

int Tmax, traiettorie;
unsigned long long seme;

if(argc != 4){
fprintf(stderr, "usage: %s <n_trajectories> <t_max> <seed>\n", argv[0]);
exit(EXIT_FAILURE);
}

traiettorie= atoi(argv[1]);
Tmax= atoi(argv[2]);
seme= strtoull(argv[3], NULL, 10);
  
  int h;
  double devstd, varianza, varianza2, devstd2;
  double radt, x;
  
  
  
    printf("# t        devstdx        x^2        devstdx^2     sqrt(t)\n");
  
  seme = (22695477*seme+1)%4294967296;
      double p = (double)seme/4294967296.0;

  
  
 for(h=0; h<traiettorie; h++){

      x= 0;
      
      for(int t=0; t<(Tmax+1); t++){
    
	if(t==0){
	x=0;
	}

      else{seme = (22695477*seme+1)%4294967296;
      double r = (double)seme/4294967296.0;


if(((x!=-25)&(x!=25))&((x!=-51)&(x!=51))&((x!=-52)&(x!=52))&((x!=-100)&(x!=100))){
      if(r>p){
	x += 1;
      }
      else {
	x -= 1;
      }
}else{

      if(r<0.9){
	x += 1;
      }
      else {
	x -= 1;
      }

}

       //valore atteso
        radt = (double)sqrt(t);
     
      varianza = (x-radt)*(x-radt)/(Tmax-1);
    devstd = (double)sqrt(varianza);
   
    varianza2 = (x*x-t)*(x*x-t)/(Tmax-1);
    devstd2 = (double)sqrt(varianza2);
  
    printf("%d  %f  %f  %f  %f %f\n", t, x, devstd, x*x, devstd2, radt);
		
	    }


	 }
printf("\n\n");

   }
   
   }
