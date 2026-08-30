#include <stdlib.h>
#include <math.h>
#include <stdio.h>



int main(int argc, char *argv[argc]){

	unsigned long long seme = 12345;   /* default seed, override with argv[3] */

	int L;
	double p;

	if(argc < 3 || argc > 4){
		fprintf(stderr, "usage: %s <lattice_size_L> <p_broken> [seed]\n", argv[0]);
		exit(EXIT_FAILURE);
		}

	L= atoi(argv[1]);
	p= atof(argv[2]);
	if(argc == 4) seme = strtoull(argv[3], NULL, 10);

	//allocazione array matrice
	int **l;
	l = (int **)malloc(L * sizeof(int*));

	for (int i = 0; i < L; i++){
		l[i] = (int *)malloc(L * sizeof(int));
	}

	if(l==NULL){
		exit(EXIT_FAILURE);
	}

	//matrice di confronto
	int **c;
	c = (int **)malloc(L * sizeof(int*));

	for (int i = 0; i < L; i++){
		c[i] = (int *)malloc(L * sizeof(int));
	}

	if(c==NULL){
		exit(EXIT_FAILURE);
	}



	int label=0;
	
	//Riempimento Reticolo linee telefoniche rotte
	for(int i=0; i<L; i++){
		for(int j=0; j<L; j++){
			seme = (16807*seme)%2147483647;
			double r1 = (double)seme/2147483647.0;
			label+=1;	
			if(r1<=p){
				l[i][j]=label*(-1);
			}else{
				l[i][j]=label;
			}

		}
	}

//inizializzazione matrice confronto
for(int i=0; i<L; i++){
  for(int j=0; j<L; j++){
   c[i][j]=l[i][j];
}
}


	//stampo reticolo linea telefonica
	printf("initial lattice\n  negative label = broken link\n  positive label = working link\n\n");

	for(int i=0; i<L; i++){
		for(int j=0; j<L; j++){
			printf("%d  ", l[i][j]);
		}
		printf("\n");
	}


//Creazione cluster
	int changes=0;
	
	do{	
		
		changes=0;	
	
for(int i=0; i<L; i++){
	for(int j=0; j<L; j++){


	  if(l[i][j]>0){
	  //sopra
	    if(i==0){
	     if(l[L-1][j]>0){	
	     	if(l[L-1][j]!=l[i][j]){
	     l[i][j]=l[L-1][j];
	     
	      }
	     }
	    }else{
	       if(l[i-1][j]>0){
	       	if(l[i-1][j]!=l[i][j]){
	       l[i][j]=l[i-1][j];
	        
	        }
	       }
	      } 
	   //giu
	   	if(i==L-1){
	     if(l[0][j]>0){
	      if(l[0][j]!=l[i][j]){
	     l[i][j]=l[0][j];
	      
	      }
	     }
	    }else{
	       if(l[i+1][j]>0){
	        if(l[i+1][j]!=l[i][j]){
	       l[i+1][j]=l[i][j];
			
	        }
	       }
	      } 
		//destra
	   	if(j==L-1){
	     if(l[i][0]>0){
          if(l[i][0]!=l[i][j]){
	     l[i][L-1]=l[i][0];
	     
	      }
	     }
	    }else{
	       if(l[i][j+1]>0){
	         if(l[i][j+1]!=l[i][j]){
	       l[i][j]=l[i][j+1];
	     
	       }
	      }
	     } 
		//sinistra
		if(j==0){	
	     if(l[i][L-1]>0){
	      if(l[i][L-1]!=l[i][j]){
	     l[i][0]=l[i][L-1];
	     
	      }
	     }
	    }else{
	       if(l[i][j-1]>0){
	        if(l[i][j-1]!=l[i][j]){
	       l[i][j]=l[i][j-1];
	     
	       }
	      } 
	    }
}

//negativi
	  if(l[i][j]<0){
	  //sopra
	    if(i==0){
	     if(l[L-1][j]<0){	
	     	if(l[L-1][j]!=l[i][j]){
	     l[i][j]=l[L-1][j];
	      
	      }
	     }
	    }else{
	       if(l[i-1][j]<0){
	       	if(l[i-1][j]!=l[i][j]){
	       l[i][j]=l[i-1][j];
	      
	        }
	       }
	      } 
	   //giu
	   	if(i==L-1){
	     if(l[0][j]<0){
	      if(l[0][j]!=l[i][j]){
	     l[i][j]=l[0][j];
	      
	      }
	     }
	    }else{
	       if(l[i+1][j]<0){
	        if(l[i+1][j]!=l[i][j]){
	       l[i][j]=l[i+1][j];
	      
	        }
	       }
	      } 
		//destra
	   	if(j==L-1){
	     if(l[i][0]<0){
          if(l[i][0]!=l[i][j]){
	     l[i][j]=l[i][0];
	      
	      }
	     }
	    }else{
	       if(l[i][j+1]<0){
	         if(l[i][j+1]!=l[i][j]){
	       l[i][j]=l[i][j+1];
	      
	       }
	      }
	     } 
		//sinistra
		if(j==0){	
	     if(l[i][L-1]<0){
	      if(l[i][L-1]!=l[i][j]){
	    l[i][j]=l[i][L-1];
	      
	      }
	     }
	    }else{
	       if(l[i][j-1]<0){
	        if(l[i][j-1]!=l[i][j]){
	       l[i][j]=l[i][j-1];
	      
	       }
	      } 
	    }
	  
	  }
	  

	
				//stampo reticolo aggiornato
		for(int i=0; i<L; i++){
			for(int j=0; j<L; j++){
				printf("%d  ", l[i][j]);
			}
			printf("\n");
		}	 
			printf("\n");
	  
}
}


		//confronto	 
		for(int i=0; i<L; i++){
			for(int j=0; j<L; j++){
   if(c[i][j]!=l[i][j]){
   changes+=1;
   }
			}
		}

		//riinizializzazione	 
		for(int i=0; i<L; i++){
			for(int j=0; j<L; j++){
            c[i][j]=l[i][j];
   
			}
		}



	} while(changes!=0);




	for(int i = 0; i<L; i++){
		free(l[i]);
	}

	free(l);

	for(int i = 0; i<L; i++){
		free(c[i]);
	}

	free(c);

	
}



