#define CL_TARGET_OPENCL_VERSION 220
#define PROGRAM_FILE "modelGARCH.cl"
#define KERNEL_FUNC "vecAdd"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <CL/cl.h>
#include "include/deviceHandler.hpp"
#include "include/kernelExecutor.hpp"
#include "include/transactionLedger.hpp"
#include "include/webSocket.hpp"

// Compile with:
// g++ -std=c++17 -O2 -o main gpu_blackScholes.cpp -lboost_system -lpthread -lOpenCL
//

int main(){
  string fName = "RandomFile";
  string Host = "binance.com";
  string port = "443";
  DeviceHandler     Dhandler;
  kernelExecutor<DeviceHandler>    executor(&Dhandler);
  TransactionLedger<DummyIOSocket> TLedger(fName);
  clientWebSocketIO<double>        webData(Host,port);

  cout << "Total devices : " << Dhandler.Get_Total_NDevs()  << endl;
  cout << "Total Cores   : " << Dhandler.Get_Total_NCores() << endl;
  return 0;
}
