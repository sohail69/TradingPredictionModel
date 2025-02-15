
// Emulates the effect of a Jacobian
//
//
void Jacobian(const double *x, double *y);

//=====================
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
// of two scaled vectors
//
//===================== 
void VecAdd(const uint M
          , const float a
          , const float b
          , const float *vec_a
          , const float *vec_b
          , float *vec_res){


}

//=====================
//
// Uses Newton-Krylov
// method to calculate
// solution to 
//
//=====================  
__kernel void newtonKrylov(__global float *u
                         , __global float *du
                         , __global const uint p
                         , __global const uint q
                         , function<>){

  int tid = get_local_id(0);
  uint Size = p+q;


  //Newton Iterations
  for(uint nIters=0; nIters<20; nIter++){


    ParaMatVec(Size, Size, JacInv, residual, du);
    VecAdd(Size, 1.0, -1.0,  u, du, u);
    PackVec(p, q, u, alphat, betat);
  }
};
