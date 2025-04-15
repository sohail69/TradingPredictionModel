#include "include/globalMacros.hpp"
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
#include "include/OrderBookParser.hpp"

// Compile with:
// g++ -std=c++17 -O2 -o main gpu_Trader.cpp -lOpenCL -lssl -lcrypto
//

int main(){
//////curl --request GET 'https://data.alpaca.markets/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD,ETH/USD,SOL/USD'
//////curl --request GET --url 'https://data.alpaca.markets/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD%2CLTC%2FUSD' \
     --header 'accept: application/json'

//  const std::string path = "/v1beta3/crypto/us";
//  const std::string host = "stream.data.alpaca.markets";
//  const std::string port = "443"; // Default port for wss

  const string path = "/v1beta3/crypto/us/latest/bars?symbols=BTC%2FUSD";
  const string host = "data.alpaca.markets";
  const string port = "443";

  string fName = "ARandledger";
//  DeviceHandler     Dhandler;
//  kernelExecutor<DeviceHandler>    executor(&Dhandler);
  TransactionLedger<DummyIOSocket> TLedger(fName);
  clientWebSocketIO<double>        webData(path,host,port);

//  cout << "Total devices : " << Dhandler.Get_Total_NDevs()  << endl;
//  cout << "Total Cores   : " << Dhandler.Get_Total_NCores() << endl;

  string body;
//  for(int I=0; I<5; I++) webData.readDatablock(body, true);

  return 0;
}
