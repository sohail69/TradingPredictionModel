#include "globalMacros.hpp"
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
    size_t program_size;
    char *program_buffer;


    void readProgramFile(const char *programFName);
    void programErrLog();
  public:
    kernelExecutor(DeviceHandler &Dhandler_, const char *programFName, plDataStruct &kernData_);
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
                                                          , const char *programFName
                                                          , plDataStruct &kernData_):
                                                          Dhandler(Dhandler_),
                                                          kernData(kernData_)
{
  cl_uint nDevs  = Dhandler.Get_Total_NDevs();
  if(nDevs != 0){
    //Construct memory buffers
    memobjs = {clCreateBuffer(Dhandler.GetContext(), CL_MEM_READ_WRITE, sizeof(plDataStruct), NULL, &clErrNum)};
    if(clErrNum==CL_SUCCESS) cout << "Success buffer created" << endl;

    // Create the compute program from the source buffer
    readProgramFile(ProgramName);
    program = clCreateProgramWithSource(Dhandler.GetContext(), 1, (const char**)&program_buffer
                                      , &program_size, &clErrNum);
    if(clErrNum==CL_SUCCESS) cout << "Success program created" << endl;
    clErrNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  }
  if(clErrNum==CL_SUCCESS) cout << "Success executor built" << endl;
  if(clErrNum==CL_BUILD_PROGRAM_FAILURE) programErrLog();
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
  if(nDevs != 0) for(unsigned I=0; I<kernels.size(); I++) clReleaseKernel(kernels[I]);
  kernels.clear();
  free(program_buffer);
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
  for(unsigned I=0; I<N; I++){
    kernels.push_back(clCreateKernel(program, (const char *) &  kernelNames[I],&clErrNum) );
    if(clErrNum==CL_SUCCESS) cout << "kernel : " << I << " built" << endl;
  }

  //Add arguments for the kernels
  for(unsigned I=0; I<N; I++){
    clSetKernelArg(kernels[I], 0, sizeof(plDataStruct),&clErrNum);
    if(clErrNum==CL_SUCCESS) cout << "kernel arg : " << I << " added" << endl;
  }
};

//
// Enqueues the configured kernels
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::runKernelsAlgorithm(){
  size_t glWorkSize = 60 * 1024;
  size_t lWorkSize = 128;

  unsigned M = kernels.size();
  unsigned N = Dhandler.Get_Total_NQueues();
  for(unsigned I=0; I<M; I++){
    for(unsigned J=0; J<N; J++){
      clEnqueueNDRangeKernel(Dhandler.GetQueue(J), kernels[I],1,NULL,&glWorkSize, &lWorkSize, 0, NULL, NULL);
    }
  }
};

//
// Write out the error log
// for a failed OpenCL program
// build
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::programErrLog(){
  // Determine the size of the log
  size_t log_size;
  clGetProgramBuildInfo(program, (Dhandler.GetDevice(0)), CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

  // Allocate memory for the log
  char *log = (char *) malloc(log_size);

  // Get the log
  clGetProgramBuildInfo(program, (Dhandler.GetDevice(0)), CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

  // Print the log
  cout << log << endl;
  free(log);
}

//
// Read in the program
// file as a string/char array
//
template<typename DeviceHandler, typename plDataStruct>
void kernelExecutor<DeviceHandler, plDataStruct>::readProgramFile(const char *programFName){
   /* Read program file and place content into buffer */
   FILE *program_handle = fopen( (const char *) & programFName, "r");
   if(program_handle == NULL)  perror("Couldn't find the program file");
   if(program_handle == NULL)  exit(1);

   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);
};
#endif


















