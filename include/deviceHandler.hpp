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
    map<cl_uint, pair<cl_platform_id,cl_device_id>>   DevIterator;
    map<pair<cl_platform_id,cl_device_id>,cl_uint>    DevNumCores;
  public:
   
    //Object constructor
    DeviceHandler();

    //Total device stats
    cl_uint Get_Total_NCores();             //Total number of cores
    cl_uint Get_Total_NDevs();              //Total number of devices
    cl_uint Get_Total_NPlats();             //Total number of platforms

    //Individual device stats
    cl_platform_id Get_PlatID(unsigned int I);     //Get the PlatID
    cl_platform_id Get_Dev_PlatID(unsigned int I); //Get the device PlatID
    cl_device_id Get_Dev_ID(unsigned int I);       //Get the device ID
    cl_uint Get_Dev_NCores(unsigned int I);        //Device number of cores
    cl_uint Get_Plat_NDevs(unsigned int I);        //Get number of devices per platform

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
  ciErrNum = clGetPlatformIDs(0, NULL, &num_platforms);

  //
  //get the number of device platform IDs
  //
  clPlatformIDs = (cl_platform_id*)malloc( num_platforms * sizeof(cl_platform_id) );
  ciErrNum = clGetPlatformIDs (num_platforms, clPlatformIDs, NULL);
  cout << setw(21) << "Num Platforms "  << setw(15)  << ": " << num_platforms << endl;

  //
  //Get number of devices for each platform
  //
  ciDeviceCounts = (cl_uint*)malloc( num_platforms * sizeof(cl_uint) );
  for(int I=0; I<num_platforms; I++){
    ciErrNum = clGetDeviceIDs(clPlatformIDs[I], CL_DEVICE_TYPE_GPU, 0, NULL, &ciDeviceCounts[I]);
    cout << setw(21) << "Ndevices Platform "+to_string(I) << setw(15) << ": " << ciDeviceCounts[I] << endl;
  }

  //
  //Get number of compute units for each device
  //
  unsigned K=0;
  for(cl_uint I=0; I<num_platforms; I++){//Go over all platforms
    cl_device_id* DevIDs_tmp;
    DevIDs_tmp = (cl_device_id*)malloc( ciDeviceCounts[I]*sizeof(cl_device_id) );
    ciErrNum = clGetDeviceIDs(clPlatformIDs[I], CL_DEVICE_TYPE_GPU, ciDeviceCounts[I], DevIDs_tmp, NULL);

    //
    // Form the maps necessary for traversing
    // the devices
    //
    for(cl_uint J=0; J<ciDeviceCounts[I]; J++){//Go over all devices per platform
      cl_uint NCUs;
      ciErrNum = clGetDeviceInfo(DevIDs_tmp[J],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(cl_uint),&NCUs,NULL);

      pair<cl_platform_id,cl_device_id> DevUID = make_pair(clPlatformIDs[I],DevIDs_tmp[J]);
      DevIterator[K]      = DevUID;
      DevNumCores[DevUID] = NCUs;
      K++;

      cout << setw(21) << "Device "+to_string(I)+"-"+to_string(J) +" NCU's" << setw(15) << ": " << NCUs 
           << setw(13) <<  DevIDs_tmp[J] << endl;
    }
    free(DevIDs_tmp);
  }
}

//
//  Object destructor
//
DeviceHandler::~DeviceHandler()
{
  free(clPlatformIDs);
  free(ciDeviceCounts);
  DevIterator.clear();
  DevNumCores.clear();
};

//
//  Total number of cores
//
cl_uint DeviceHandler::Get_Total_NCores(){
  cl_uint Total_NCores=0;
  for(unsigned int I=0; I<DevIterator.size(); I++){
    Total_NCores = Total_NCores + DevNumCores[DevIterator[I]];
  }
  return Total_NCores;
};

//
//  Total number of platforms
//
cl_uint DeviceHandler::Get_Total_NPlats(){
  return num_platforms;
};

//
//  Total number of devices
//
cl_uint DeviceHandler::Get_Total_NDevs(){
  return DevIterator.size();
};


//
//  Get the PlatID
//
cl_platform_id DeviceHandler::Get_PlatID(unsigned int I){
  return clPlatformIDs[I];
}


//
//  Get the device PlatID
//
cl_platform_id DeviceHandler::Get_Dev_PlatID(unsigned int I){
  return DevIterator[I].first;
}


//
//  Get the device ID
//
cl_device_id DeviceHandler::Get_Dev_ID(unsigned int I){
  return DevIterator[I].second;
};


//
//  Device number of cores
//
cl_uint DeviceHandler::Get_Dev_NCores(unsigned int I){
  return DevNumCores[DevIterator[I]];
};

//
// //Get number of devices per platform
//
cl_uint DeviceHandler::Get_Plat_NDevs(unsigned int I){
  return ciDeviceCounts[I];
};
#endif
