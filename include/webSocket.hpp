#include <iostream>
#include <string>
#include <functional>

// Include Boost libraries
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

// Template function to connect and read from a WebSocket
template <typename Handler>
void connectAndReadWebSocket(const std::string& host,
                             const std::string& port,
                             const std::string& target,
                             Handler handleMessage)
{
    try
    {
        // The io_context is required for all I/O
        asio::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        websocket::stream<tcp::socket> ws(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        asio::connect(ws.next_layer(), results.begin(), results.end());

        // Perform the WebSocket handshake
        ws.handshake(host, target);

        std::cout << "Connected to WebSocket server at " << host << target << std::endl;

        // Buffer to hold incoming messages
        beast::flat_buffer buffer;

        while (true)
        {
            // Read a message
            ws.read(buffer);

            // Convert buffer to a string message
            std::string message = beast::buffers_to_string(buffer.data());

            // Handle the message using the provided handler
            handleMessage(message);

            // Clear the buffer for the next read
            buffer.consume(buffer.size());
        }

        // Close the WebSocket connection (unreachable in this loop)
        ws.close(websocket::close_code::normal);
    }
    catch (const beast::system_error& se)
    {
        std::cerr << "Error: " << se.what() << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
}
