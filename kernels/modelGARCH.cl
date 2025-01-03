
//=====================
//
// Exponential function
//
//=====================
float MyExp(float x){ return exp(x);};

//=====================
//
// SQRT function
//
//=====================
float MySqrt(float x){ return sqrt(x);};

//=====================
//
// Normal distribution
// function
//
//=====================
float NormalDist(float Val, float Mean, float Var){
  float Pi = 3.14159265359;
  N = 1.0/MySqrt(2.0*Pi*Var);
  N = N*MyExp( -(Val-Mean)*(Val-Mean)/(2.0*Var) );
  return N;
}

float RandGen(float state){
  return ;
}

float sigma_t2(){

  return sigmat2;
};



__kernel void GARCH_pq(__global const float *stockHistory
                      , __global const uint nStocks
                      , __global const uint nHistory
                      , __global float *stockPrediction){

  int tid = get_local_id(0);
  uint t;
  float etat=0.0, sigmat2=0.0, alpha=0.0, beta=0.0, omega=0.0;
  float zt=0.0, mu=0.0, Rt=0.0;
  float randNum=0.0;
  
  randNum = RandGen(randNum);
  sigmat2 = omega + alpha*sigmat2 + beta*etat;
  zt = NormalDist(randNum,mean,sigmat2);
  etat = MySqrt
  Rt = mu + etat
};

__kernel void GARCH_calcCoeffs(__global const float *stockHistory
                             , __global const uint nStocks
                             , __global const uint nHistory
                             , __global float *alphat
                             , __global float *betat){

  int tid = get_local_id(0);



};

void GARCH_calcResiduals(const uint p
                       , const uint q
                       , const float sigmat02
                       , const float omega
                       , const float *alphat
                       , const float *betat
                       , const float *sigmat2
                       , const float *epsilont2
                       , float *residual_alpha
                       , float *residual_beta){

  //Calculate the energy functional
  float Pi = sigmat02 - omega;
  for(uint I=0; I<p; I++)
    Pi = Pi - alphat[I]*sigmat2[I];

  for(uint I=0; I<q; I++)
    Pi = Pi - betat[I]*epsilont2[I];

  //Calculate the residuals
  for(uint I=0; I<p; I++)
    residual_alpha[I] = 2.0*Pi*sigmat2[I];

  for(uint I=0; I<q; I++)
    residual_beta[I] = 2.0*Pi*epsilont2[I];
};











