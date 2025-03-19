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
!    4.4) if(trend doesn't varies from previous trend) EXIT
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
// mpic++ -std=c++17 -O2 -o GPUMarketFitter GPU_PassiveDataFitter.cpp -fopenmp -lssl -lcrypto
// -lOpenCL

#include <functional>
#include <iostream>
#include <iomanip>
#include <mpi.h>
#include <omp.h>

//Interface class to all necessary objects
//and definitions
#include "include/GPU_PassiveDataFitter_Dummy.hpp"

using namespace std;

int IteratorFunc(const int& I){ return I + 1;};

int main(){
  const int I=0;
  const unsigned int N=100;
  double x_vec[N], y_vec[N], b_vec[N];
  int num_devices = omp_get_num_devices();
  int nthreads0 = omp_get_num_threads();
  function<int(const int& I)> IterFuncPtr(IteratorFunc);
  cout << setw(10) << num_devices << setw(10) << nthreads0 << endl;

  #pragma omp target
  {
    int nteams = omp_get_num_teams();
    int nthreads = omp_get_num_threads();
    int J = IterFuncPtr(I);
  }
  return 0;
}


