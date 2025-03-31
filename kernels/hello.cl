
struct pDstruct{
  //model and data work
  static const unsigned int p=15, q=15; //GARCH (p,q) model size
  static const unsigned int nData=100;  //Size of the dataset
  double alpha_beta[p+q];

  //Newton solver parameters
  double du[p+q], residual[p+q];
  double Jacobian[p+q][p+q];
};


__kernel void helloWorld(__global pDstruct data){
  uint tid = get_local_id(0);
  if(tid < (data.p + data.q)){
    data.du[tid] = residual[tid];
    data.alpha_beta[tid] =   data.alpha_beta[tid] + data.du[tid];
  }
}
