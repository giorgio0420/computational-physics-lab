#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(int argc, char *argv[argc]){

int Tmax, traiettorie, x;
unsigned long long seme = 12345;   /* default seed, override with argv[3] */

if(argc < 3 || argc > 4){
fprintf(stderr, "usage: %s <n_trajectories> <t_max> [seed]\n", argv[0]);
exit(EXIT_FAILURE);
}

traiettorie= atoi(argv[1]);
Tmax= atoi(argv[2]);
if(argc == 4) seme = strtoull(argv[3], NULL, 10);
  
  int h;
  double devstd, varianza, varianza2, devstd2;
  double radt;
  
  
  
    printf("# t        devstdx        x^2        devstdx^2     sqrt(t)\n");
  
 for(h=0; h<traiettorie; h++){

      x= 0;
      
      for(int t=0; t<(Tmax+1); t++){
    
	if(t==0){
	x=0;
	}
	
      else{seme = (16807*seme)%2147483647;
      double r = (double)seme/2147483647.0;

      if(r>0.5){
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
  
    printf("%d  %d  %f  %d  %f %f\n", t, x, devstd, x*x, devstd2, radt);
		
	    }

printf("\n\n");

	 }

   }
