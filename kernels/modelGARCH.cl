
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



__kernel void GARCH_1_1(__global const float *stockHistory
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
  uint t;
  float etat=0.0, sigmat2=0.0, alpha=0.0, beta=0.0, omega=0.0;
  float zt=0.0, mu=0.0, Rt=0.0;
  float randNum=0.0;

};
