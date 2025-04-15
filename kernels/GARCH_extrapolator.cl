/****************************************************\
!  Extra templated maths functions
!
!  Basic maths functions used for calculations
!  of different things
!
\****************************************************/  

// Exponential function
float MyExp(float x){ return exp(x); };


// SQRT function
float MySqrt(float x){ return sqrt(x); };


// Normal distribution function
float NormalDist(float Val, float Mean, float Var){
  float Pi = 3.14159265359;
  N = 1.0/MySqrt(2.0*Pi*Var);
  N = N*MyExp( -(Val-Mean)*(Val-Mean)/(2.0*Var) );
  return N;
}

// Iterators for I and J
unsigned iteratorI(unsigned l, unsigned P){return l/P;};

unsigned iteratorJ(unsigned l, unsigned P){
  unsigned M = l/P;
  return (l - M*P);
};

/****************************************************\
!  RNG calculations
!
!  RNG used for random sampling
!
\****************************************************/  
unsigned int _pcg6432_uint(pcg6432_state* state){
  unsigned long int oldstate = *state;
  *state = oldstate * 6364136223846793005UL + 0xda3e39cb94b95bdbUL;
	uint xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
#define pcg6432_double(state) (pcg6432_ulong(state)*PCG6432_DOUBLE_MULTI)


/****************************************************\
!  GARCH calculations
!
!  Calculates the GARCH coefficients
!
\****************************************************/  
void GARCH_calcResiduals(const uint p
                       , const uint q
                       , const float sigmat02
                       , const float omega
                       , const float *alphat
                       , const float *betat
                       , const float *sigmat2
                       , const float *epsilont2
                       , float *residual){

  //Calculate the energy functional
  float Pi = sigmat02 - omega;
  for(uint I=0; I<p; I++)
    Pi = Pi - alphat[I]*sigmat2[I];

  for(uint I=0; I<q; I++)
    Pi = Pi - betat[I]*epsilont2[I];

  //Calculate the residuals
  for(uint I=0; I<p; I++)
    residual[I] = 2.0*Pi*sigmat2[I];

  for(uint I=0; I<q; I++)
    residual[I+p] = 2.0*Pi*epsilont2[I];
};

//=====================
//
// Calculate the Jacobian
//
//=====================
void GARCH_calcJacobians(const uint p
                       , const uint q
                       , const float *sigmat2
                       , const float *epsilont2
                       , float **Jac){

  //Calculate the Jacobian block Matrices
  uint iLower = 0;
  uint iUpper = p*p;
  uint iSize  = iUpper - iLower;
  for(uint K=0; K<iSize; K++){
    uint I = iteratorI(K,p);
    uint J = iteratorJ(K,p);
    Jac[I+0][J+0] = 2.0*sigmat2[I]*sigmat2[J];
  }

  iLower = p*p;
  iUpper = p*(p+q);
  iSize  = iUpper - iLower;
  for(uint K=0; K<iSize; K++){
    uint I = iteratorI(K,q);
    uint J = iteratorJ(K,q);
    Jac[I+p][J+0] = 2.0*sigmat2[I]*epsilont2[J];
    Jac[J+0][I+p] = Jac[I][J];
  }

  iLower = (p+q)*(p+q) - q*q;
  iUpper = (p+q)*(p+q);
  iSize  = iUpper - iLower;
  for(uint K=0; K<iSize; K++){
    uint I = iteratorI(K,q);
    uint J = iteratorJ(K,q);
    Jac[I+p][J+p] = 2.0*epsilont2[I]*epsilont2[J];
  }
};


//=====================
//
// Calculate the Inverse
// Matrix using
// LU-factorisation
//
//=====================
void LU_factor(const uint N
             , const float **Amat
             , float **LUMat){

  for(uint I=1; I<N; I++){
    for(uint K=1; K<N; K++){
      LUMat[I][K] = Amat[I][K];
    }

    for(uint K=1, I<(I-1); I++){
      LUMat[I][K] = Amat[I][K]/Amat[K][K];
      for(uint J=0; J<(K+1); J++){
        LUMat[I][J] = LUMat[I][J] - LUMat[I][K]*LUMat[K][J];
      }
    }
  }
};


//=====================
//
// Calculate the matrix
// vector product
//
//===================== 
void ParaMatVec(const uint M
              , const uint N
              , const float **Mat
              , const float *Vec
              , float *MatVec){

  for(uint I=0; I<N; I++) MatVec[I] = 0.0;
  for(uint K=0; K<(M*N); K++){
    uint I = iteratorI(K,M);
    uint J = iteratorJ(K,M);
    MatVec[I] += Mat[I][J]*Vec[J]
  }
}

//=====================
//
// Calculate the sum
// of two scaled vectors
//
//===================== 
void VecAdd(const uint M
          , const float a
          , const float b
          , const float *vec_a
          , const float *vec_b
          , float *vec_res){

  for(uint K=0; K<M; K++) vec_res[K]=a*vec_a[K] + b*vec_b[K];
}

//=====================
//
// Unpack or pack the
// vector to/from
// two sub vectors
//
//===================== 
void UnpackVec(const uint M
             , const uint N
             , const float *vec_res
             , float *vec_alpha
             , float *vec_beta){

  for(uint K=0; K<M; K++)
    vec_alpha[K]=vec_res[K];

  for(uint K=0; K<N; K++)
    vec_beta[K]=vec_res[K+M];
}

void PackVec(const uint M
             , const uint N
             , const float *vec_res
             , float *vec_alpha
             , float *vec_beta){

  if(M != 0)
    for(uint K=0; K<M; K++)
      vec_res[K]=vec_alpha[K];

  if(N != 0)
    for(uint K=0; K<N; K++)
      vec_res[K+M]=vec_beta[K];
}


//=====================
//
// Uses Newtons method
// to calculate the
// GARCH coefficients
//
//=====================  
__kernel void GARCH_calcCoeffs(__global const float *stockHistory
                             , __global const uint nStocks
                             , __global const uint nHistory
                             , __global float *alphat
                             , __global float *betat){

  int tid = get_local_id(0);
  uint Size = p+q;
  //Newton Iterations
  GARCH_calcJacobians(p, q, sigmat2, epsilont2, Jac);


  for(uint nIters=0; nIters<20; nIter++){
    UnpackVec(p, q, u, alphat, betat);
    GARCH_calcResiduals(p, q, sigmat02, omega, alphat, betat, sigmat2, epsilont2, residual);
    ParaMatVec(Size, Size, JacInv, residual, du);
    VecAdd(Size, 1.0, -1.0,  u, du, u);
    PackVec(p, q, u, alphat, betat);
  }
};
