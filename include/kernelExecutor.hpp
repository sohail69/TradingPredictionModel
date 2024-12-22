#define CL_TARGET_OPENCL_VERSION 220
#ifndef KERNELEXECUTOR_HPP
#define KERNELEXECUTOR_HPP

//
// 1-context per platform
// 1-quene per device
//


#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <CL/cl.h>

using namespace std;

template<typename DeviceHandler>
class kernelExecutor{
  private:
    DeviceHandler      *Dhandler;   
    cl_int             ciErrNum;
    vector<cl_context> devContexts;
  public:
    kernelExecutor(DeviceHandler *Dhandler_);

    void addKernel(string kernelName, unsigned int Chronoglogy);
    void setKernel(string kernelName);


    ~kernelExecutor();
};

template<typename DeviceHandler>
kernelExecutor<DeviceHandler>::kernelExecutor(DeviceHandler *Dhandler_){
  Dhandler = Dhandler_;
  cl_uint nPlats = Dhandler->Get_Total_NPlats();
  cl_device_id *DevIDs;
  devContexts = vector<cl_context>(nPlats);


  for(cl_uint J=0; J<nPlats; J++){
    //Create a single Device context
    cl_uint nDevs  = Dhandler->Get_Plat_NDevs(J);

    if(nDevs != 0){ 
      DevIDs = new cl_device_id[nDevs];
      cl_platform_id PlatID = Dhandler->Get_PlatID(J);

      cl_uint K=0;
      for(cl_uint I=0; I<nDevs; I++){
        cl_platform_id DevPlatID = Dhandler->Get_Dev_PlatID(I);
        if(DevPlatID==PlatID){
          cout << setw(10) << I << setw(10) << K << endl;
          DevIDs[K] = Dhandler->Get_Dev_ID(I);
          K++;
        }
      }
      //devContexts[J] = clCreateContext(NULL, nDevs, DevIDs, NULL, NULL, &ciErrNum);
      cl_context devContext = clCreateContext(NULL, nDevs, &DevIDs[0], NULL, NULL, &ciErrNum);
      if(ciErrNum==CL_SUCCESS) cout << "Great Executor Success" << endl;
      delete[] DevIDs;
    }
  }
};

template<typename DeviceHandler>
kernelExecutor<DeviceHandler>::~kernelExecutor(){
};

#endif
