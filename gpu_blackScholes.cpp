#define CL_TARGET_OPENCL_VERSION 220
#define PROGRAM_FILE "vecAdd.cl"
#define KERNEL_FUNC "vecAdd"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <CL/cl.h>
#include "include/deviceHandler.hpp"

// Compile with:
// g++ -o main gpu_blackScholes.cpp -lOpenCL
//

int main(){
  DeviceHandler Dhandler;
  return 0;
}
