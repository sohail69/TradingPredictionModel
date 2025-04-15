#include "OCLVersion.h"
#ifndef DEVICEHANDLER_HPP
#define DEVICEHANDLER_HPP 


#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <CL/cl.h>

//
// The device handler manages all the
// devices on the local computer and
// allocates work to them
//
using namespace std;

class DeviceHandler
{
  private:
    cl_int                     clErrNum;
    cl_uint                    TNumCores, clDevCount, num_platforms;
    cl_platform_id             clPlatformID;
    cl_context                 GPUContext;
    vector<cl_device_id>       DevIDs;
    map<cl_device_id,cl_uint>  DevNumCores;
    vector<cl_command_queue>   Queues;
  public:

    //Object constructor
    DeviceHandler();

    //Total device stats
    cl_context &GetContext();   //Gets the context
    cl_uint Get_Total_NCores(); //Total number of cores
    cl_uint Get_Total_NDevs();  //Total number of devices
    cl_uint Get_Total_NQueues();//Total number of device queues

    //Individual device stats
    cl_uint Get_Dev_NCores(unsigned I);     //Gets a devices number of cores
    cl_command_queue &GetQueue(unsigned I); //Gets a command queue

    //Object destructor
    ~DeviceHandler();
};

//
//  Object constructor
//
DeviceHandler::DeviceHandler()
{
  //
  //Get the number of device platforms
  //
  clErrNum = clGetPlatformIDs(0, NULL, &num_platforms);
  TNumCores=0;
  clDevCount=0;

  if(num_platforms != 0){
    //
    //Get the platformID and deviceIDs
    //
    clErrNum = clGetPlatformIDs (1, &clPlatformID, NULL);
    clErrNum = clGetDeviceIDs(clPlatformID, CL_DEVICE_TYPE_GPU, 0, NULL, &clDevCount);

    //
    //Get number of compute units for each device
    //
    unsigned K=0;
    cl_device_id* DevIDs_tmp;
    DevIDs_tmp = (cl_device_id*)malloc( clDevCount*sizeof(cl_device_id) );
    clErrNum = clGetDeviceIDs(clPlatformID, CL_DEVICE_TYPE_GPU, clDevCount, DevIDs_tmp, NULL);

    //
    // Form the maps necessary for traversing
    // the devices
    //
    DevIDs.clear();
    DevNumCores.clear();
    for(cl_uint J=0; J<clDevCount; J++){//Go over all devices
      cl_uint NCUs;
      clErrNum = clGetDeviceInfo(DevIDs_tmp[J],CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(cl_uint),&NCUs,NULL);
      DevIDs.push_back(DevIDs_tmp[J]);
      DevNumCores[DevIDs_tmp[J]] = NCUs;

      TNumCores = TNumCores + NCUs;
    }

    //
    // Build the context and
    // command queues
    //
    GPUContext = clCreateContext(0, clDevCount, DevIDs_tmp, NULL, NULL, &clErrNum);
    for(cl_uint J=0; J<clDevCount; J++){//Go over all devices
      Queues.push_back(clCreateCommandQueue(GPUContext, DevIDs[J], CL_QUEUE_PROFILING_ENABLE, &clErrNum));
    }
    free(DevIDs_tmp);
  }
  cout << setw(15)  << num_platforms << " := " << setw(25) << "Num Platforms "           << endl;
  cout << setw(15)  << clDevCount    << " := " << setw(25) << "Num of platform devices " << endl;
  cout << setw(15)  << TNumCores     << " := " << setw(25) << "Num of total cores "      << endl;
  if(clErrNum==CL_SUCCESS) cout << "Success Device handler Built" << endl;
}

//
//  Object destructor
//
DeviceHandler::~DeviceHandler()
{
  if(num_platforms != 0){
    for(int I=0; I<Queues.size(); I++) clReleaseCommandQueue(Queues[I]);
    clReleaseContext(GPUContext);
  }
  Queues.clear();
  DevNumCores.clear();
  DevIDs.clear();
};

//
// Total number of cores
//
cl_uint DeviceHandler::Get_Total_NCores(){
  return TNumCores;
};

//
// Total number of devices
//
cl_uint DeviceHandler::Get_Total_NDevs(){
  return clDevCount;
};

//
// Device number of cores
//
cl_uint DeviceHandler::Get_Dev_NCores(unsigned I){
  return  DevNumCores[DevIDs[I]];
};

//
// Total number of device queues
//
cl_uint DeviceHandler::Get_Total_NQueues(){
  return  Queues.size();
};


//
// Gets a command queue
//
cl_command_queue &DeviceHandler::GetQueue(unsigned I){
  return  Queues[I];
};

//
//Gets the context
//
cl_context &DeviceHandler::GetContext(){
  return GPUContext;
};
#endif
