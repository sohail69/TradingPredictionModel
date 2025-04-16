/****************************************************\
!  Passive Market Data fitter
!
!  Approximates coefficients in market data passively
!  for a given stochatstic model with a random variable
!  (GARCH[p,q] in this case), it does this by:
!  1) setting up (a) web socket(s) collecting data
!     (possibly in parallel)
!  2) The data is partitioned, triply device-node level
!     and node-node level and each subdomain partition
!     is broken up into training and testing set
!  3) The model is trained initially on the training set
!     using a standard optimiser
!  4) Loop 
!    4.1) Loop
!      4.1.1) The model is tested on the test-set using 
!             stochastic extrapolator and to approximate error
!      4.1.2) The model is re-trained on the training set
!             using a standard optimiser
!    4.2) Calculate coefficient shift with neighboring
!         partitions
!    4.3) register trend
!    4.4) if(trend doesn't vary from previous trend) EXIT
!  5) Output Model Coeffs
!  6) Output Coefficient shift Distributions
!
!  Key Functions
!  -NodeLevelPartition(tstart, tend, increment)                     RESULT(tstartL, tendL)
!  -WebsocketReader(tstartL, tendL, increment)                      RESULT(NodeLocalBlockVectors)
!  -GlobalNodeDeviceDataPartitioner(nDevices,NodeLocalBlockVectors) RESULT(DeviceLocalBlockVectors)
!  -ModelResidualFunction(DeviceLocalBlockVectors,CoeffGuess)       RESULT(modelCoeffResidual)
!  -ModelOptimiser(ModelResidualFunction, DeviceLocalBlockVectors)  RESULT(modelCoeffs)
!  -ModelPredictorFunction(modelCoeffs,randomVar,prevData)          RESULT(FutureDataPoint)
!  -ModelStochasticExtrapolator(modelCoeffs,ModelPredictorFunction) RESULT(FutureData)
!  -CoefficientShiftCalc(modelCoeffs, ModelShifts)                  RESULT(ModelShifts)
!  -Output Dist(Data)                                               RESULT()
\****************************************************/

// Compile with
// mpic++ -std=c++17 -O2 -o GPUMarketFitter GPU_PassiveDataFitter.cpp -lOpenCL -lssl -lcrypto
// g++ -std=c++17 -O2 -o GPUMarketFitter GPU_PassiveDataFitter.cpp -lOpenCL -lssl -lcrypto -lnvidia-opencl
#include <functional>
#include <iostream>
#include <iomanip>
//#include <mpi.h>

//Interface class to all necessary objects
//and definitions
#include "include/deviceHandler.hpp"
#include "include/kernelExecutor.hpp"
#include "include/globalMacros.hpp"

using namespace std;

struct PACKSTRUCT pDstruct{
  //model and data work
  static const unsigned int p=15, q=15; //GARCH (p,q) model size
  static const unsigned int nData=100;  //Size of the dataset
  double sigma2[nData], epsilon2[nData];
  double alpha_beta[p+q];

  //Newton solver parameters
  double du[p+q], residual[p+q];
  double Jacobian[p+q][p+q], JacInv[p+q][p+q];
};

int main(){
  const int I=0;
  string progName="kernels/hello.cl";
  pDstruct pData;

  DeviceHandler dhandler;
  kernelExecutor<DeviceHandler, pDstruct> kex(dhandler, progName, pData);
  kex.AddKernel(make_pair(0,"helloWorld"));
  kex.prepareForExecution();
//  kex.runKernelsAlgorithm();
  return 0;
}


