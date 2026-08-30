#include <stdlib.h>
#include <math.h>
#include <stdio.h>

int main(){

unsigned long long seme;

int L=4;
int Tmax =100, t=0;
double p=0.7;

//allocazione array matrice
int **l;
l = (int **)malloc(L * sizeof(int*));
for (int i = 0; i < L; i++){
  l[i] = (int *)malloc(L * sizeof(int));
}
int *x;
x = (int *)malloc(L*L * sizeof(int));
int *y;
y = (int *)malloc(L*L * sizeof(int));


if(l==NULL){
exit(EXIT_FAILURE);
}
if(x==NULL){
exit(EXIT_FAILURE);
}
if(y==NULL){
exit(EXIT_FAILURE);
}

int label=0;



//riempimento Reticolo Gas 
for(int i=0; i<L; i++){
for(int j=0; j<L; j++){
      seme = (16807*seme)%2147483647;
      srand(seme);
      int random = rand();    
      double r1 = (double)random/RAND_MAX;

if(r1<=p){
label+=1;
l[i][j]=label;
x[l[i][j]]=i;
y[l[i][j]]=j;
}else{
l[i][j]=0;
x[l[i][j]]=i;
y[l[i][j]]=j;
}

}
}

//stampo reticolo gas
printf("initial lattice\n  1 = occupied\n  0 = empty\n\n");

for(int i=0; i<L; i++){
for(int j=0; j<L; j++){
printf("%d  ", l[i][j]);
}
printf("\n");
}

int rx,ry;

do{
t+=1;

int direzione;

do{
seme = (16807*seme)%2147483647;
      srand(seme);
      rx = rand()%L+1;    
seme = (16807*seme)%2147483647;
      srand(seme);
     // random = rand()%L+1;    
      //ry= random/RAND_MAX*L;
      ry= rand()%L+1;

printf("%d %d %d %d\n", rx, ry, RAND_MAX, L);

}while(l[rx][ry]!=0);


      seme = (16807*seme)%2147483647;
      srand(seme);
      int random = rand();    
      double r = (double)random/RAND_MAX;

printf("%f\n",r);

//sopra
if(r<0.25){
direzione =1;
}
//destra
if((r<=0.25)&(r<0.5)){
direzione =2;
}
//giu
if((r>=0.5)&(r<0.75)){
direzione =3;
}
//sinistra
if(r>=0.75){
direzione =4;
}

if(direzione==1){
if(y[l[rx][ry]]==L-1){
if(l[rx][0]==0){
l[rx][0]=1;
l[rx][ry]=0;
x[l[rx][0]]=rx;
y[l[rx][0]]=0;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}else{
if(l[rx][ry+1]==0){
l[rx][ry+1]=1;
l[rx][ry]=0;
x[l[rx][ry+1]]=rx;
y[l[rx][ry+1]]=ry+1;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}
}

if(direzione==2){
if(x[l[rx][ry]]==L-1){
if(l[0][ry]==0){
l[0][ry]=1;
l[rx][ry]=0;
x[l[0][ry]]=0;
y[l[0][ry]]=ry;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}else{
if(l[rx+1][ry]==0){
l[rx+1][ry]=1;
l[rx][ry]=0;
x[l[rx+1][ry]]=rx+1;
y[l[rx+1][ry]]=ry;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}
}

if(direzione==3){
if(y[l[rx][ry]]==0){
if(l[rx][L-1]==0){
l[rx][L-1]=1;
l[rx][0]=0;
x[l[rx][0]]=rx;
y[l[rx][0]]=0;
x[l[rx][L-1]]=rx;
y[l[rx][L-1]]=L-1;
}
}else{
if(l[rx][ry-1]==0){
l[rx][ry-1]=1;
l[rx][ry]=0;
x[l[rx][ry-1]]=rx;
y[l[rx][ry-1]]=ry-1;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}
}

if(direzione==4){
if(x[l[rx][ry]]==0){
if(l[L-1][ry]==0){
l[L-1][ry]=1;
l[rx][ry]=0;
x[l[L-1][ry]]=L-1;
y[l[L-1][ry]]=ry;
x[l[0][ry]]=0;
y[l[0][ry]]=ry;
}
}else{
if(l[rx-1][ry]==0){
l[rx-1][ry]=1;
l[rx][ry]=0;
x[l[rx-1][ry]]=rx-1;
y[l[rx-1][ry]]=ry;
x[l[rx][ry]]=rx;
y[l[rx][ry]]=ry;
}
}
}

}while(t>Tmax);

//stampo reticolo gas
for(int i=0; i<L; i++){
printf("\n");
for(int j=0; j<L; j++){
printf("%d  ", l[i][j]);
}

}











for (int i = 0; i < L; i++) {
  free(l[i]);
}
free(l);
free(x);
free(y);



}
