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
#include <string>
#include <vector>
#include <map>
#include <CL/cl.h>

using namespace std;

template<typename DeviceHandler>
class kernelExecutor{
  private:
    DeviceHandler            *Dhandler;   
    cl_int                    ciErrNum;
    vector<string>            kernelNames;
    vector<cl_context>        devContexts;
    vector<cl_command_queue>  devQueues;
  public:
    ~kernelExecutor();
    kernelExecutor(DeviceHandler *Dhandler_);

    void addKernel(string kernelName, unsigned int Chronoglogy);
    void setKernels();
    void runKernels();
};

template<typename DeviceHandler>
kernelExecutor<DeviceHandler>::~kernelExecutor(){
  devContexts.clear();
  kernelNames.clear();
  devContexts.clear();
  devQueues.clear();
};


template<typename DeviceHandler>
kernelExecutor<DeviceHandler>::kernelExecutor(DeviceHandler *Dhandler_){
  Dhandler = Dhandler_;
  cl_uint nPlats = Dhandler->Get_Total_NPlats();
  cl_device_id *DevIDs;
  devContexts = vector<cl_context>(nPlats);
  devQueues.clear();

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
        }
      }
      //devContexts[J] = clCreateContext(NULL, nDevs, DevIDs, NULL, NULL, &ciErrNum);
      cl_context devContext = clCreateContext(NULL, nDevs, DevIDs, NULL, NULL, &ciErrNum);

      for(cl_uint I=0; I<nDevs; I++){
        cl_command_queue_properties props;
        cl_command_queue queue = clCreateCommandQueueWithProperties(devContext, DevIDs[I], &props, &ciErrNum);
        devQueues.push_back(queue);
      }
      if(ciErrNum==CL_SUCCESS) cout << "Great Success Executor Built" << endl;
      delete[] DevIDs;
    }
  }
};


template<typename DeviceHandler>
void kernelExecutor<DeviceHandler>::addKernel(string kernelName, unsigned int Chronoglogy){

};

template<typename DeviceHandler>
void kernelExecutor<DeviceHandler>::setKernels(){
 /*
  cl_mem memobjs[] = {clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * 2 * NUM_ENTRIES, NULL, NULL), clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * 2 * NUM_ENTRIES, NULL, NULL)};

  // create the compute program
  // const char* fft1D_1024_kernel_src[1] = {  };
  cl_program program = clCreateProgramWithSource(context, 1, (const char **)& KernelSource, NULL, NULL);

  // build the compute program executable
  clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  // create the compute kernel
  cl_kernel kernel = clCreateKernel(program, "fft1D_1024", NULL);
*/
};

template<typename DeviceHandler>
void kernelExecutor<DeviceHandler>::runKernels(){
  
};
#endif


