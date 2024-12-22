#define CL_TARGET_OPENCL_VERSION 220
#define PROGRAM_FILE "vecAdd.cl"
#define KERNEL_FUNC "vecAdd"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <CL/cl.h>
#include "include/deviceHandler.hpp"
#include "include/kernelExecutor.hpp"

// Compile with:
// g++ -o main gpu_blackScholes.cpp -lOpenCL
//

int main(){
  DeviceHandler  Dhandler;
  kernelExecutor<DeviceHandler> executor(&Dhandler);

  cout << "Total devices : " << Dhandler.Get_Total_NDevs()  << endl;
  cout << "Total Cores   : " << Dhandler.Get_Total_NCores() << endl;

  return 0;
}
