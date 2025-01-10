// alpaca_websocket_client.cpp

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
#include <array>
#include <random>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

// Function to generate a random Sec-WebSocket-Key
std::string generate_websocket_key() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789+/";
    std::string key;
    key.reserve(16);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    for (int i = 0; i < 16; ++i) {
        key += charset[dis(gen)];
    }
    // Base64 encode the 16-byte key
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/";
    std::string encoded;
    for (size_t i = 0; i < key.size(); i += 3) {
        int val = 0;
        int valb = -6;
        for (size_t j = 0; j < 3 && (i + j) < key.size(); ++j) {
            val = (val << 8) + static_cast<unsigned char>(key[i + j]);
            valb += 8;
        }
        while (valb >= 0) {
            encoded += base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
        if (i + 1 >= key.size()) {
            encoded += "==";
        } else if (i + 2 >= key.size()) {
            encoded += "=";
        }
    }
    return encoded;
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

// Function to generate a random masking key
std::array<unsigned char, 4> generate_masking_key() {
    std::array<unsigned char, 4> key;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for(auto &byte : key) {
        byte = static_cast<unsigned char>(dis(gen));
    }
    return key;
}

// Function to send data with WebSocket framing (text frames only)
std::string websocket_frame(const std::string& message) {
    std::string frame;
    frame += static_cast<char>(0x81); // FIN bit set and text frame

    size_t payload_len = message.size();
    if (payload_len <= 125) {
        frame += static_cast<char>(0x80 | payload_len); // Mask bit set
    }
    else if (payload_len <= 65535) {
        frame += static_cast<char>(0x80 | 126);
        uint16_t len = htons(static_cast<uint16_t>(payload_len));
        frame.append(reinterpret_cast<char*>(&len), sizeof(len));
    }
    else {
        frame += static_cast<char>(0x80 | 127);
        uint64_t len = htobe64(static_cast<uint64_t>(payload_len));
        frame.append(reinterpret_cast<char*>(&len), sizeof(len));
    }

    // Generate and append masking key
    auto masking_key = generate_masking_key();
    frame.append(reinterpret_cast<char*>(masking_key.data()), masking_key.size());

    // Mask the payload
    std::string masked_payload = message;
    for (size_t i = 0; i < masked_payload.size(); ++i) {
        masked_payload[i] ^= masking_key[i % 4];
    }

    frame += masked_payload;
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
            payload_length = ntohs(*reinterpret_cast<uint16_t*>(extended));
        }
        else if (payload_length == 127) {
            unsigned char extended[8];
            boost::asio::read(ssl_socket, boost::asio::buffer(extended, 8));
            payload_length = be64toh(*reinterpret_cast<uint64_t*>(extended));
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

        // Handle control frames
        if (opcode == 0x8) { // Connection Close Frame
            std::cout << "Received close frame from server.\n";
            return "";
        }
        else if (opcode == 0x9) { // Ping Frame
            // Respond with Pong
            std::string pong_frame = "\x8A\x00";
            boost::asio::write(ssl_socket, boost::asio::buffer(pong_frame));
            return receive_websocket_frame(ssl_socket); // Continue receiving
        }
        else if (opcode == 0xA) { // Pong Frame
            // Pong responses can be ignored or used to verify connection liveness
            return receive_websocket_frame(ssl_socket); // Continue receiving
        }
        else if (opcode == 0x1) { // Text Frame
            return message;
        }
        else {
            // Handle other opcodes if necessary
            return "";
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception in receive_websocket_frame: " << e.what() << "\n";
        return "";
    }
}

int main(int argc, char* argv[]) {
    try {
     /*   if (argc != 3) {
            std::cerr << "Usage: alpaca_websocket_client <API_KEY_ID> <SECRET_KEY>\n";
            return 1;
        }

        std::string api_key = argv[1];
        std::string secret_key = argv[2];*/
        std::string api_key    ="PKZLV5L2XPYAFM5HVTLO";
        std::string secret_key ="4zLDE7vbS1ABhNWlFNCJLgdeqsXyp45f3va0q8AO";

        // Initialize Boost.Asio
        boost::asio::io_context io_context;

        // Create an SSL context
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ssl::verify_peer);

        // Create and connect the socket
        tcp::resolver resolver(io_context);
        ssl::stream<tcp::socket> ssl_socket(io_context, ctx);

        // Resolve the Alpaca WebSocket endpoint
//        std::string host = "stream.data.alpaca.markets";
        std::string host ="https://paper-api.alpaca.markets/V2/PA308C24F51F";
        std::string port = "443";
        auto endpoints = resolver.resolve(host, port);

        // Connect to the endpoint
        boost::asio::connect(ssl_socket.lowest_layer(), endpoints);

        // **Set SNI Hostname here**
        if (!SSL_set_tlsext_host_name(ssl_socket.native_handle(), host.c_str())) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }

        // Perform SSL handshake
        ssl_socket.handshake(ssl::stream_base::client);

        // Generate WebSocket handshake headers
        std::string ws_key = generate_websocket_key();
        std::string sec_accept_expected = generate_sec_accept(ws_key);

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
        boost::algorithm::trim(sec_accept_expected); // Remove any trailing whitespace
        boost::algorithm::trim(sec_accept_response);

        if (sec_accept_response != sec_accept_expected) {
            std::cerr << "Sec-WebSocket-Accept mismatch\n";
            return 1;
        }

        std::cout << "WebSocket handshake successful!\n";

        // Prepare authentication message (JSON string)
        std::string auth_str = "{\"action\":\"auth\",\"key\":\"" + api_key + "\",\"secret\":\"" + secret_key + "\"}";

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

        // Prepare subscription message (JSON string)
        // Modify the channels and symbols based on Alpaca's API documentation
        std::string subscribe_str = "{\"action\":\"subscribe\",\"trades\":[\"AAPL\",\"GOOG\"]}";

        // Send subscription message
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
            if (message.empty()) {
                std::cerr << "Connection closed or error occurred.\n";
                break; // Connection closed or error
            }
            std::cout << "Received message: " << message << "\n";
            // Implement further processing as needed
        }

        // Close the connection gracefully by sending a close frame
        std::string close_frame = "\x88\x00"; // FIN bit set, opcode 0x8 (Close), no payload
        boost::asio::write(ssl_socket, boost::asio::buffer(close_frame));

        // Perform SSL shutdown
        boost::system::error_code ec;
        ssl_socket.shutdown(ec);
        if (ec) {
            std::cerr << "SSL shutdown failed: " << ec.message() << "\n";
        }

    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
