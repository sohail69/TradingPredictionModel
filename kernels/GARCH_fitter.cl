/****************************************************\
!  Extra templated maths functions
!
!  Basic maths functions used for calculations
!  of different things
!
\****************************************************/
__kernel void Increment(__global pDstruct data){
  uint tid = get_local_id(0);
  if(tid < (data.p + data.q)){
    data.du[tid] = residual[tid];
    data.alpha_beta[tid] = data.alpha_beta[tid] + data.du[tid];
  }
}

__kernel void Matvec(__global pDstruct data){
  uint I=0, N=(data.p + data.q);
  uint tid = get_local_id(0);
  if(tid < (data.p + data.q)){
    data.du[tid] = 0.00;
    for(I=0; I<N; I++) data.du[tid] =  data.du[tid] + JacInv[tid][I]*residual[I];
  }
}
