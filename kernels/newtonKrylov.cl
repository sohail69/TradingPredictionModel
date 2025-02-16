
// Residual function
//
void Res(const double *x, double *y, const unsigned M);
                   
/****************************************************\
!  The kernel executor:
!
!  Takes a device handler which holds records of local
!  machine devices and supplied Kernels to construct 
!  a Kernel executor which decides how kernels are 
!  executed and on which devices
!
\****************************************************/
//
// Iterators for I
// and J
//
//=====================
uint iteratorI(uint l, uint P){ 
  return l/P;
};

uint iteratorJ(uint l, uint P){
  uint M = l/P;
  return (l - M*P);
};


//=====================
//
// Calculate the sum
// of two vectors
//
//===================== 
void VecAdd(float       *vec_a
          , const float *vec_b
          , const float  a
          , const float  b
          , const uint   M){
  #pragma unroll
  for(int I=0; I<M; I++) vec_a[I] = a*vec_a[I] + b*vec_b[I];
}



/****************************************************\
!  The Flexible-GMRES solver:
!
!  Uses Flexible-GMRES method to solve a linear/NL
!  problem potentionally preconditioned with
!  FD-Jacobian approximatiion
!
\****************************************************/  
void FGMRES( float *u
           , float *du
           , const uint M
           , const uint
           , const float 
           , void(*Res)(const double *x, double *y, const unsigned M) ){

  int tid = get_local_id(0);




};


/****************************************************\
!  Finite difference Jacobian Increment action
!
!  Approximates the jacobian Increment matrix
!  vector product for usage in the Newton-Krylov
!  method using only 
!
!
\****************************************************/  
void FD_Jacobian(double *Jac_du
               , double *u
               , const double *du
               , const double *Res
               , const unsigned M){

  double sigma=1.0E-7;
  VecAdd(u_temp, du, 1.0, sigma, M);
  (*Res)(u_temp, Jac_du, M);
  VecAdd(u_temp, du, 1.0, sigma, M);

};

/****************************************************\
!  The Newton-Krylov solver:
!
!  Uses Newton-Krylov method to solve a vector
!  non-linear problem
!
\****************************************************/ 
__kernel void newtonKrylov(__global float *u
                         , __global float *du
                         , __global const uint M
                         , __global const uint
                         , __global const float 
                         , void(*Res)(const double *x, double *y, const unsigned M) ){

  int tid = get_local_id(0);
  uint Size = p+q;


  //Newton Iterations
  for(uint nIters=0; nIters<20; nIter++){
    (*Jac)()
    VecAdd(u, du, 1.0, -1.0, M);
  }
};
