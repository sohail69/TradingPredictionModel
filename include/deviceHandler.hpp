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
    cl_uint                    clDevCount, num_platforms;
    cl_platform_id             clPlatformID;
    vector<cl_device_id>       DevIDs;
    map<cl_device_id,cl_uint>  DevNumCores;
    cl_context                 GPUContext;
    vector<cl_command_queue>   Queues;
  public:
   
    //Object constructor
    DeviceHandler();

    //Total device stats
    cl_uint Get_Total_NCores();             //Total number of cores
    cl_uint Get_Total_NDevs();              //Total number of devices

    //Individual device stats
    cl_device_id Get_Dev_ID(unsigned int I);   //Get the device ID
    cl_uint Get_Dev_NCores(unsigned int I);    //Device number of cores
    cl_uint Get_Plat_NDevs(unsigned int I);    //Get number of devices per platform

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
     // clErrNum = clGetDeviceInfo(DevIDs_tmp[J],CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(cl_uint),&NCUs,NULL);

      DevIDs.push_back(DevIDs_tmp[J]);
     // DevNumCores[DevIDs_tmp[J]] = NCUs;
    }
    free(DevIDs_tmp);

    //
    // Build the context and
    // command queues
    //
    GPUContext = clCreateContext(0, clDevCount, DevIDs_tmp, NULL, NULL, &clErrNum);
    for(cl_uint J=0; J<clDevCount; J++){//Go over all devices
      Queues.push_back(clCreateCommandQueue(GPUContext, DevIDs[J], CL_QUEUE_PROFILING_ENABLE, &clErrNum);
    }
  }
  cout << setw(21) << "Num Platforms "  << setw(15)  << ": " << num_platforms << endl;
}

/*
    cl_int   clErrNum, clDevCount;
    cl_platform_id             clPlatformID;
    vector<cl_device_id>       DevIDs;
    map<cl_device_id,cl_uint>  DevNumCores;
    vector<cl_command_queue>   Queues;
*/

//
//  Object destructor
//
DeviceHandler::~DeviceHandler()
{
  DevIDs.clear();
  DevNumCores.clear();
};

#endif
