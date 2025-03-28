#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define L 60

int ip[L], im[L];

 //Periodic Boundary conditions
void pbc(){
  for (int i = 0; i < L; i++){
    ip[i] = (i+1)%L;
    im[i] = (i-1+L)%L;
  }
}

double randu(double x, double y){
  return (double)rand()/(double)RAND_MAX * (y-x) + x; 
}

void HotStart(int spin[L][L]){
  // Hot Start
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      if( randu(0.0,1.0) < 0.5){
	spin[i][j] = 1;
	  }
      else{
	spin[i][j] = -1;
      }
    }
  }
}


void ColdStart(int spin[L][L]){
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      spin[i][j] = 1;
    }
  }
}

void PrintConfiguration(int spin[L][L]){
  //Print Configuration
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      printf("%2d  ",spin[i][j]);
    }
    printf("\n");
  }
}



//Energy difference
int DH(int spin[L][L], int i, int j){
  return 2*spin[i][j] * (spin[ip[i]][j] + spin[im[i]][j] + spin[i][ip[j]] + spin[i][im[j]]);
}


void Metropolis(int spin[L][L],int i, int j, double T){
  int DE;

  DE = DH(spin,i,j);
  if( DE <= 0){
    spin[i][j] *= -1;
  }
  else{
    if( randu(0.0,1.0) <= exp(-(double)DE/T)){
      spin[i][j] *= -1;
    }
  }
}

double EnergyDensity(int spin[L][L]){
  int E;
  E = 0;
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      E += spin[i][j] * (spin[ip[i]][j] + spin[i][ip[j]]);
    }
  }
  return -(double)E/(double)(L*L);
}

double MagnetizationDensity(int spin[L][L]){
  int M;
  M = 0;
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      M += spin[i][j];
    }
  }
  return (double)abs(M)/(double)(L*L);
}


void sweeps(int spin[L][L], double T){
  for (int i = 0; i < L; i++){
    for (int j = 0; j < L; j++){
      Metropolis(spin,i,j, T);
    }
  }
}

double avr(int N, double E[N]){
  double sum;
  sum = 0.0;
  for (int i = 0; i < N; i++){
    sum += E[i];
  }
  return sum/N;
}

double var(int N, double E[N]){
  double sum, avg;
  sum = 0.0;
  avg = avr(N,E);
  for (int i = 0; i < N; i++){
    sum += (E[i] - avg)*(E[i] - avg);
  }
  return sum/(N-1);
}

double std_err(int N, double E[N]){
  return sqrt(var(N,E)/N);
}

int main(){  
  
  int spin[L][L];
  //const int Nt = 10, Nskip = 10, Nmeas = 1000, Nterm = 500;
  double Tmin = 0.1, Tmax = 4.0;
  
  int Nt, Nskip, Nmeas, Nterm;
  //double T[Nt], E[Nmeas];
 
  
  
  FILE *DataFile = fopen("measurements.dat","w");
  FILE *ParametersFile = fopen("parameters.txt","r");

  
  fscanf(ParametersFile,"Nt=%d\n",&Nt);
  fscanf(ParametersFile,"Nskip=%d\n",&Nskip);
  fscanf(ParametersFile,"Nmeas=%d\n",&Nmeas);
  fscanf(ParametersFile,"Nterm=%d\n",&Nterm);

  printf("L = %d\n", L);
  printf("Nt = %d\n",Nt);    
  printf("Nskip = %d\n",Nskip);
  printf("Nmeas = %d\n",Nmeas);
  printf("Nterm = %d\n",Nterm);
  
  double* T = (double*) malloc(Nt * sizeof(double));
  double* E = (double*) malloc(Nmeas * sizeof(double));
  double* M = (double*) malloc(Nmeas * sizeof(double));
    

  srand(time(NULL));

  //pbc
  pbc();
 
  ColdStart(spin);
  
  for (int i = 0; i < Nt; i++){
    T[i] = Tmin + (Tmax - Tmin)/(Nt - 1) * i;
  }



  for (int it = 0; it < Nt; it++){
    // Thermalization
    for (int i_sweep = 0; i_sweep < Nterm; i_sweep++){
      sweeps(spin,T[it]);
    }

    // Measurements
    for (int i_sweep = 0; i_sweep < Nmeas; i_sweep++){
      for( int i_skip = 0; i_skip < Nskip; i_skip++){sweeps(spin,T[it]);}
      E[i_sweep] = EnergyDensity(spin);
      M[i_sweep] = MagnetizationDensity(spin);
    }
    fprintf(DataFile,"%.15f %.15f %.15f %.15f %.15f\n",T[it],avr(Nmeas,E),std_err(Nmeas,E),avr(Nmeas,M),std_err(Nmeas,M));
    
  }
  free(T);
  free(E);
  return 0;
}

