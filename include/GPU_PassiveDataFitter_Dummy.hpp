#ifndef GPU_PASSIVEDATAFITTER_DUMMY_HPP
#define GPU_PASSIVEDATAFITTER_DUMMY_HPP
#include <vector>
#include <array>
#include <string>

using namespace std;

/****************************************************\
!  Key Functions
!  -NodeLevelPartition(tstart, tend, increment)                     RESULT(tstartL, tendL)
!  -WebsocketReader(tstart, tend, increment)                        RESULT(NodeLocalBlockVectors)
!  -GlobalNodeDeviceDataPartitioner(nDevices,NodeLocalBlockVectors) RESULT(DeviceLocalBlockVectors)
!  -CoefficientShiftCalc(modelCoeffs)                               RESULT()
!  -Output Dist(Data)                                               RESULT()
\****************************************************/
void WebsockRead();


/****************************************************\
!  Key Functions
!  -NodeLevelPartition(tstart, tend, increment)                     RESULT(tstartL, tendL)
!  -WebsocketReader(tstart, tend, increment)                        RESULT(NodeLocalBlockVectors)
!  -GlobalNodeDeviceDataPartitioner(nDevices,NodeLocalBlockVectors) RESULT(DeviceLocalBlockVectors)
!  -CoefficientShiftCalc(modelCoeffs)                               RESULT()
!  -Output Dist(Data)                                               RESULT()
\****************************************************/
template<class WSocket, unsigned nDevices>
class parallelIOMultiSocket{
  private:
    

  public:
    parallelIOMultiSocket(std::string InputFName){};

    void NodeLevelPartition();
    void GlobalNodeDeviceDataPartitioner(vector<double>                 NodeLocalBlockVectors
                                       , vector<array<double,nDevices>> DeviceLocalBlockVectors){};

    void CoefficientShiftCalc(vector<double> modelCoeffs){};
//    void Output Dist(Data);
/*
!  Key Functions
!  -NodeLevelPartition(tstart, tend, increment)                     RESULT(tstartL, tendL)
!  -WebsocketReader(tstart, tend, increment)                        RESULT(NodeLocalBlockVectors)
!  -GlobalNodeDeviceDataPartitioner(nDevices,NodeLocalBlockVectors) RESULT(DeviceLocalBlockVectors)
!  -CoefficientShiftCalc(modelCoeffs)                               RESULT()
!  -Output Dist(Data)                                               RESULT()
*/

};

/****************************************************\
!  Key Functions
!  -ModelResidualFunction(DeviceLocalBlockVectors,CoeffGuess)       RESULT(modelCoeffResidual)
!  -ModelPredictorFunction(modelCoeffs,randomVar,prevData)          RESULT(FutureDataPoint)
\****************************************************/  



/****************************************************\
!  Key Functions
!  -ModelOptimiser(ModelResidualFunction, DeviceLocalBlockVectors)  RESULT(modelCoeffs)
\****************************************************/  


/****************************************************\
!  Key Functions
!  -ModelStochasticExtrapolator(modelCoeffs,ModelPredictorFunction) RESULT(FutureData
\****************************************************/  
#endif
