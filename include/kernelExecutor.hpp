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
    cl_program            programs;
    map<unsigned,string>  kernelNames;

  public:
    kernelExecutor(DeviceHandler &Dhandler_, string ProgramName, plDataStruct &kernData_);
    ~kernelExecutor();

    void addKernel(pair<unsigned,string> kernels);
    void addKernels(vector<pair<unsigned,string>> kernels);
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
/*
  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &clErrNum);
  checkError(clErrNum, "Creating program");

  // Build the program  
  clErrNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
*/
  if((nDevs != 0)and(clErrNum==CL_SUCCESS)) cout << "Success Executor Built" << endl;
};

//
// Class destructor
//
template<typename DeviceHandler, typename plDataStruct>
kernelExecutor<DeviceHandler, plDataStruct>::~kernelExecutor(){
  kernelNames.clear();
};


//
// Adds a kernel program and
// its necessary chronology
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::addKernel(pair<unsigned,string> kernels){

};

//
// Adds a set of kernel programs and
// their necessary chronology
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::addKernels(vector<pair<unsigned,string>> kernels){

};


template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::prepareForExecution(){
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

template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::runKernelsAlgorithm(){
  
};
#endif


