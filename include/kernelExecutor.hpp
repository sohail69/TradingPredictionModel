#include "OCLVersion.h"
#ifndef KERNELEXECUTOR_HPP
#define KERNELEXECUTOR_HPP

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

/****************************************************\
!  The kernel executor:
!
!  Takes a device handler which holds records of local
!  machine devices and supplied Kernels to construct 
!  a Kernel executor which decides how kernels are 
!  executed and on which devices
!
\****************************************************/
template<typename DeviceHandler, typename plDataStruct>
class kernelExecutor{
  private:
    DeviceHandler &       Dhandler;
    plDataStruct  &       kernData;
    cl_int                clErrNum;
    cl_program            program;
    cl_mem                memobjs;
    map<unsigned,string>  kernelNames;
    vector<cl_kernel>     kernels;

  public:
    kernelExecutor(DeviceHandler &Dhandler_, string ProgramName, plDataStruct &kernData_);
    ~kernelExecutor();

    void addKernel(pair<unsigned,string> kernel_);
    void addKernels(vector<pair<unsigned,string>> kernels_);
    void prepareForExecution();
    void runKernelsAlgorithm();
};

/****************************************************\
!  The kernel executor Implementation:
!
!  addKernel : add additional program+chronology
!  setKernels: sets Kernel memory an queues before exec
!  runKernels: runs kernels on device(s)
!
\****************************************************/
//
// Performs a kernel execution
// event that is prescheduled
// and configured
//
template<typename DeviceHandler, typename plDataStruct>
kernelExecutor<DeviceHandler, plDataStruct>::kernelExecutor(DeviceHandler &Dhandler_
                                                          , string ProgramName
                                                          , plDataStruct &kernData_):
                                                          Dhandler(Dhandler_),
                                                          kernData(kernData_)
{
  cl_uint nDevs  = Dhandler.Get_Total_NDevs();
  if(nDevs != 0){
    //Construct memory buffers
    memobjs = {clCreateBuffer(Dhandler.GetContext(), CL_MEM_READ_WRITE, sizeof(plDataStruct), NULL, &clErrNum)};

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(Dhandler.GetContext(), 1, (const char **) & ProgramName, NULL, &clErrNum);
    clErrNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  }
  if(clErrNum==CL_SUCCESS) cout << "Success Executor Built" << endl;
};

//
// Class destructor
//
template<typename DeviceHandler, typename plDataStruct>
kernelExecutor<DeviceHandler, plDataStruct>::~kernelExecutor(){
  kernelNames.clear();
  clReleaseMemObject(memobjs);
  cl_uint nDevs  = Dhandler.Get_Total_NDevs();
  if(nDevs != 0) clReleaseProgram(program);

  ////clReleaseKernel(kernel1);
};


//
// Adds a kernel program and
// its necessary chronology
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::addKernel(pair<unsigned,string> kernel_){
  kernelNames[kernel_.first] = kernel_.second;
};


//
// Adds a set of kernel programs and
// their necessary chronology
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::addKernels(vector<pair<unsigned,string>> kernels_){
  for(unsigned I=0; I<kernels.size(); I++) kernelNames[kernels_[I].first] = kernels_[I].second;
};

//
// Prepares the kernels for enqueuing
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::prepareForExecution(){
  if(kernels.size() != 0){ //clear existing kernels
    for(unsigned I=0; I<kernels.size(); I++) clReleaseKernel(kernels[I]);
    kernels.clear();
  }

  //Construct the kernels
  unsigned N = kernelNames.size();
  for(unsigned I=0; I<N; I++) 
    kernels.push_back(clCreateKernel(program, (const char *) &  kernelNames[I],&clErrNum) );

  //Add arguments for the kernels
  for(unsigned I=0; I<N; I++)
    clSetKernelArg(kernels[I], 0, sizeof(plDataStruct),&clErrNum);
};

//Dhandler.GetQueue(I);

//
// Enqueues the configured kernels
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::runKernelsAlgorithm(){
  unsigned M = kernels.size();
  unsigned N = Dhandler.Get_Total_NQueues();
  for(unsigned I=0; I<M; I++){
    for(unsigned J=0; J<N; J++){
      clEnqueueNDRangeKernel(Dhandler.GetQueue(J), kernels[I],1,NULL,&"TODO", &"TODO", 0, NULL, NULL);
    }
  }
};
#endif


















