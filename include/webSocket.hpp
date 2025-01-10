#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP 

// Include standard libraries
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>

// Include Boost libraries
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/algorithm/string.hpp>

// Include OpenSSL libraries
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

//
//
// Template function to connect and read from a WebSocket
//
//
template<typename dType>
class clientWebSocketIO{
  private:
    string target, host, port;

    void throwException();
  public:
    clientWebSocketIO(const std::string& host, const std::string& port);

    void readDatablock(dType* T, float tWait, int nSkip, int Blocksize, int BlockLength);

    ~clientWebSocketIO();
};

//Dummmy implementation
template<typename dType>
clientWebSocketIO<dType>::clientWebSocketIO(const std::string& host, const std::string& port){};

//Dummmy implementation
template<typename dType>
void clientWebSocketIO<dType>::readDatablock(dType* T, float tWait, int nSkip, int Blocksize, int BlockLength){};

//Dummmy implementation
template<typename dType>
clientWebSocketIO<dType>::~clientWebSocketIO(){};

#endif
