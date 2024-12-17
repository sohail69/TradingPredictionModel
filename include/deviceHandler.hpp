#define CL_TARGET_OPENCL_VERSION 220
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
    cl_int   ciErrNum;
    cl_uint  num_platforms; 
    cl_uint* ciDeviceCounts;
    cl_platform_id* clPlatformIDs;
    vector<cl_uint> uiTargetDevice;
    map<pair<cl_platform_id,cl_uint>,cl_device_id> DevIDs;
    map<pair<cl_platform_id,cl_device_id>,cl_uint> DevNumCores;

  public:
   
    DeviceHandler();

    

    ~DeviceHandler();
};


DeviceHandler::DeviceHandler()
{
  //Get the number of device platforms
  ciErrNum = clGetPlatformIDs(0, NULL, &num_platforms);

  //get the number of device platform IDs
  clPlatformIDs = (cl_platform_id*)malloc( num_platforms * sizeof(cl_platform_id) );
  ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
  cout << setw(19) << "Num Platforms "  << setw(15)  << ": " << num_platforms << endl;

  //Get number of devices for each platform
  ciDeviceCounts = (cl_uint*)malloc( num_platforms * sizeof(cl_uint) );
  for(int I=0; I<num_platforms; I++){
    ciErrNum = clGetDeviceIDs(clPlatformIDs[I], CL_DEVICE_TYPE_GPU, 0, NULL, &ciDeviceCounts[I]);
    cout << setw(19) << "Ndevices Platform "+to_string(I) << setw(15) << ": " << ciDeviceCounts[I] << endl;
  }

  //Get number of compute units for each device
  for(cl_uint I=0; I<num_platforms; I++){//Go over all platforms
    cl_uint NumComputeUnits;
    cl_device_id DevID;
    ciErrNum = clGetDeviceIDs(clPlatformIDs[I], CL_DEVICE_TYPE_GPU, uiNumDevices, DevID, NULL);
    ciErrNum = clGetDeviceInfo(DevID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), NumComputeUnits, NULL);
 
    for(cl_uint J=0; J<ciDeviceCounts[I]; J++){//Go over all devices per platform
      DevIDs[make_pair(clPlatformIDs[I],J)]          = DevID;
      DevNumCores[make_pair(clPlatformIDs[I],DevID)] = NumComputeUnits;
    }

  }
}

DeviceHandler::~DeviceHandler()
{
  free(clPlatformIDs);
  free(ciDeviceCounts);
};
#endif
