/*
    // Boost.Asio I/O context
    asio::io_context ioc;

    // WebSocket server details
    std::string host = "wss://stream.data.sandbox.alpaca.markets/v1beta3/crypto/us";
    std::string port = "443"; // Use "443" for secure WebSockets (wss)
    std::string target = "/";
*/
// alpaca_websocket_client.cpp

//g++ -std=c++11 -I /path/to/boost/include -I /path/to/json/include -o alpaca_websocket_client alpaca_websocket_client.cpp -lboost_system -lssl -lcrypto -lpthread

//g++ -std=c++11 -o main testWebsocket.cpp -lboost_system -lssl -lcrypto -lpthread


#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
//#include <nlohmann/json.hpp> // For JSON handling

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
//using json = nlohmann::json;

// Function to generate a random Sec-WebSocket-Key
std::string generate_websocket_key() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789+/";
    std::string key;
    key.reserve(16);
    for (int i = 0; i < 16; ++i) {
        key += charset[rand() % (sizeof(charset) - 1)];
    }
    return key;
}

// Function to perform Base64 encoding
std::string base64_encode(const unsigned char* input, int length) {
    BIO* bmem = nullptr;
    BIO* b64 = nullptr;
    BUF_MEM* bptr = nullptr;

    b64 = BIO_new(BIO_f_base64());
    // Do not use newlines to flush buffer
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    std::string encoded(bptr->data, bptr->length);
    BIO_free_all(b64);

    return encoded;
}

// Function to generate Sec-WebSocket-Accept from Sec-WebSocket-Key
std::string generate_sec_accept(const std::string& key) {
    std::string GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string concatenated = key + GUID;

    // SHA1 hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(concatenated.c_str()), concatenated.length(), hash);

    // Base64 encode
    return base64_encode(hash, SHA_DIGEST_LENGTH);
}

// Function to send data with WebSocket framing (simplified for text frames)
std::string websocket_frame(const std::string& message) {
    std::string frame;
    frame += 0x81; // FIN bit set and text frame

    if (message.size() <= 125) {
        frame += static_cast<char>(message.size());
    }
    else if (message.size() <= 65535) {
        frame += static_cast<char>(126);
        uint16_t len = htons(static_cast<uint16_t>(message.size()));
        frame.append(reinterpret_cast<char*>(&len), sizeof(len));
    }
    else {
        frame += static_cast<char>(127);
        uint64_t len = htobe64(static_cast<uint64_t>(message.size()));
        frame.append(reinterpret_cast<char*>(&len), sizeof(len));
    }

    frame += message;
    return frame;
}

// Function to receive a single WebSocket frame
std::string receive_websocket_frame(boost::asio::ssl::stream<tcp::socket>& ssl_socket) {
    try {
        // Read the first two bytes
        unsigned char header[2];
        boost::asio::read(ssl_socket, boost::asio::buffer(header, 2));

        bool fin = header[0] & 0x80;
        int opcode = header[0] & 0x0F;
        bool masked = header[1] & 0x80;
        uint64_t payload_length = header[1] & 0x7F;

        // Read extended payload length if necessary
        if (payload_length == 126) {
            unsigned char extended[2];
            boost::asio::read(ssl_socket, boost::asio::buffer(extended, 2));
            payload_length = ntohs(*(uint16_t*)extended);
        }
        else if (payload_length == 127) {
            unsigned char extended[8];
            boost::asio::read(ssl_socket, boost::asio::buffer(extended, 8));
            payload_length = be64toh(*(uint64_t*)extended);
        }

        // Read masking key if masked (Note: Server frames are not masked)
        unsigned char masking_key[4];
        if (masked) {
            boost::asio::read(ssl_socket, boost::asio::buffer(masking_key, 4));
        }

        // Read payload data
        std::vector<char> payload(payload_length);
        boost::asio::read(ssl_socket, boost::asio::buffer(payload.data(), payload_length));

        // Unmask if necessary
        if (masked) {
            for (size_t i = 0; i < payload_length; ++i) {
                payload[i] ^= masking_key[i % 4];
            }
        }

        std::string message(payload.begin(), payload.end());

        return message;
    }
    catch (std::exception& e) {
        std::cerr << "Exception in receive_websocket_frame: " << e.what() << "\n";
        return "";
    }
}

int main(int argc, char* argv[]) {
    try {
        std::string api_key    ="PKZLV5L2XPYAFM5HVTLO";
        std::string secret_key ="4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO";

        // Initialize Boost.Asio
        boost::asio::io_context io_context;

        // Create an SSL context
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_default_verify_paths();

        // Create and connect the socket
        tcp::resolver resolver(io_context);
        ssl::stream<tcp::socket> ssl_socket(io_context, ctx);

        // Resolve the Alpaca WebSocket endpoint
//        std::string host = "stream.data.alpaca.markets/v1beta3/crypto/us";
        std::string host = "https://paper-api.alpaca.markets/v2";
        std::string port = "443";
        auto endpoints = resolver.resolve(host, port);

        // Connect to the endpoint
        boost::asio::connect(ssl_socket.lowest_layer(), endpoints);


        // Perform SSL handshake
        ssl_socket.handshake(ssl::stream_base::client);

        // Generate WebSocket handshake headers
        std::string ws_key = generate_websocket_key();
        std::string ws_accept = generate_sec_accept(ws_key);

        std::string request =
            "GET /v2/iex HTTP/1.1\r\n"
            "Host: " + host + "\r\n" +
            "Upgrade: websocket\r\n" +
            "Connection: Upgrade\r\n" +
            "Sec-WebSocket-Key: " + ws_key + "\r\n" +
            "Sec-WebSocket-Version: 13\r\n\r\n";

        // Send the handshake request
        boost::asio::write(ssl_socket, boost::asio::buffer(request));

        // Read the handshake response
        boost::asio::streambuf response;
        boost::asio::read_until(ssl_socket, response, "\r\n\r\n");

        // Process the handshake response
        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        std::string status_message;

        response_stream >> http_version >> status_code;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cerr << "Invalid handshake response\n";
            return 1;
        }

        if (status_code != 101) {
            std::cerr << "Handshake failed with status code " << status_code << "\n";
            return 1;
        }

        // Read headers
        std::string header;
        std::string sec_accept_response;
        while (std::getline(response_stream, header) && header != "\r") {
            if (boost::istarts_with(header, "Sec-WebSocket-Accept:")) {
                sec_accept_response = header.substr(22); // Length of "Sec-WebSocket-Accept:"
                boost::algorithm::trim(sec_accept_response);
            }
        }

        // Validate Sec-WebSocket-Accept
        std::string expected_accept = generate_sec_accept(ws_key);
        boost::algorithm::trim(expected_accept);

        if (sec_accept_response != expected_accept) {
            std::cerr << "Sec-WebSocket-Accept mismatch\n";
            return 1;
        }

        std::cout << "WebSocket handshake successful!\n";

        // Prepare authentication message
/*        json auth_msg = {
            {"action", "auth"},
            {"key", api_key},
            {"secret", secret_key}
        };*/
//        std::string auth_str = auth_msg.dump();
        std::string auth_str = "{{\"action\", \"auth\"},{\"key\","
                             + api_key
                             + "},{\"secret\"," 
                             + secret_key
                             + "}}";

        // Send authentication message
        std::string framed_auth = websocket_frame(auth_str);
        boost::asio::write(ssl_socket, boost::asio::buffer(framed_auth));

        // Receive authentication response
        std::string auth_response = receive_websocket_frame(ssl_socket);
        if (auth_response.empty()) {
            std::cerr << "Failed to receive authentication response\n";
            return 1;
        }

        std::cout << "Authentication response: " << auth_response << "\n";

        // Example: Subscribe to a specific data stream after authentication
        // Modify as per Alpaca's API documentation
        // For example, to subscribe to trade updates:
/*        json subscribe_msg = {
            {"action", "subscribe"},
            {"trades", {"AAPL", "GOOG"}}
        };*/
//        std::string subscribe_str = subscribe_msg.dump();
        std::string subscribe_str = "{ {\"action\",\"subscribe\"}, {\"trades\", {\"AAPL\", \"GOOG\"}}";

        std::string framed_subscribe = websocket_frame(subscribe_str);
        boost::asio::write(ssl_socket, boost::asio::buffer(framed_subscribe));

        // Receive subscription response
        std::string subscribe_response = receive_websocket_frame(ssl_socket);
        if (subscribe_response.empty()) {
            std::cerr << "Failed to receive subscription response\n";
            return 1;
        }
        std::cout << "Subscription response: " << subscribe_response << "\n";

        // Start receiving data in a loop
        std::cout << "Listening for incoming messages...\n";
        while (true) {
            std::string message = receive_websocket_frame(ssl_socket);
            if (message.empty()) break; // Connection closed or error
            std::cout << "Received message: " << message << "\n";
            // Implement further processing as needed
        }

        // Close the connection gracefully if needed
        // Note: Implementing a proper close handshake is recommended

    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
