// websocket_client.cpp

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

// Namespaces for convenience
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

// Thread-safe queue for outgoing messages
template<typename T>
class SafeQueue {
public:
    void enqueue(T msg) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(std::move(msg));
        cond_var_.notify_one();
    }

    bool dequeue(T& msg) {
        std::unique_lock<std::mutex> lock(mtx_);
        while (queue_.empty()) {
            if (finished_) return false;
            cond_var_.wait(lock);
        }
        msg = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void set_finished() {
        std::lock_guard<std::mutex> lock(mtx_);
        finished_ = true;
        cond_var_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cond_var_;
    bool finished_ = false;
};

// WebSocketClient class handling high-frequency data
class WebSocketClient : public std::enable_shared_from_this<WebSocketClient> {
public:
    WebSocketClient(asio::io_context& ioc,
                   const std::string& host,
                   const std::string& port,
                   const std::string& target)
        : resolver_(asio::make_strand(ioc)),
          ws_(asio::make_strand(ioc)),
          host_(host),
          port_(port),
          target_(target),
          is_connected_(false)
    {}

    // Start the connection process
    void run() {
        resolver_.async_resolve(host_, port_,
            beast::bind_front_handler(&WebSocketClient::on_resolve, shared_from_this()));
    }

    // Enqueue a message to be sent
    void send(const std::string& message) {
        outgoing_messages_.enqueue(message);
        // Post to the strand to ensure thread safety
        asio::post(ws_.get_executor(),
            beast::bind_front_handler(&WebSocketClient::do_write, shared_from_this()));
    }

    // Close the WebSocket connection
    void close() {
        asio::post(ws_.get_executor(),
            beast::bind_front_handler(&WebSocketClient::do_close, shared_from_this()));
    }

    // Handle incoming messages with a user-provided callback
    void set_message_handler(std::function<void(const std::string&)> handler) {
        message_handler_ = handler;
    }

private:
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string port_;
    std::string target_;
    SafeQueue<std::string> outgoing_messages_;
    std::function<void(const std::string&)> message_handler_;
    std::atomic<bool> is_connected_;
    // Resolver handler
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) fail(ec, "resolve");
        if (ec) return;

        // Make the connection on the IP address we get from a lookup
        asio::async_connect(ws_.next_layer(), results.begin(), results.end(),
            beast::bind_front_handler(&WebSocketClient::on_connect, shared_from_this()));
    }

    // Connect handler
    void on_connect(beast::error_code ec, const tcp::endpoint& endpoint) {
        if(ec) fail(ec, "connect");
        if(ec) return;

        // Update the host_ string to include the port
        // (required by the WebSocket handshake)
        std::string host_with_port = host_ + ":" + std::to_string(endpoint.port());

        // Perform the WebSocket handshake
        ws_.async_handshake(host_with_port, target_,
            beast::bind_front_handler(&WebSocketClient::on_handshake, shared_from_this()));
    }

    // Handshake handler
    void on_handshake(beast::error_code ec) {
        if (ec) {
            fail(ec, "handshake");
            return;
        }

        is_connected_ = true;
        std::cout << "Connected to WebSocket server at " << host_ << target_ << std::endl;

        // Start reading messages
        do_read();

        // Start writing messages if any
        do_write();
    }

    // Read handler
    void do_read() {
        ws_.async_read(buffer_,
            beast::bind_front_handler(&WebSocketClient::on_read, shared_from_this()));
    }

    // Read completion handler
    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec == websocket::error::closed) {
            std::cout << "WebSocket connection closed by server." << std::endl;
            return;
        }

        if (ec) {
            fail(ec, "read");
            return;
        }

        // Convert the buffer to a string
        std::string message = beast::buffers_to_string(buffer_.data());

        // Handle the message using the user-provided callback
        if (message_handler_) {
            message_handler_(message);
        }

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Continue reading
        do_read();
    }

    // Write handler
    void do_write() {
        if (!is_connected_) return;

        std::string msg;
        if (outgoing_messages_.dequeue(msg)) {
            // Ensure that we're not already writing
            if (is_writing_) {
                // Re-enqueue the message for later
                outgoing_messages_.enqueue(msg);
                return;
            }

            is_writing_ = true;

            // Send the message asynchronously
            ws_.text(true); // Set to text mode; use ws_.binary(true) for binary
            ws_.async_write(asio::buffer(msg),
                beast::bind_front_handler(&WebSocketClient::on_write, shared_from_this()));
        }
    }

    // Write completion handler
    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            fail(ec, "write");
            return;
        }

        is_writing_ = false;

        // Attempt to write the next message
        do_write();
    }

    // Close handler
    void do_close() {
        if (!is_connected_) {
            return;
        }

        ws_.async_close(websocket::close_code::normal,
            beast::bind_front_handler(&WebSocketClient::on_close, shared_from_this()));
    }

    // Close completion handler
    void on_close(beast::error_code ec) {
        if (ec) {
            fail(ec, "close");
            return;
        }
        std::cout << "WebSocket connection closed gracefully." << std::endl;
    }

    // Error handling function
    void fail(beast::error_code ec, char const* what) {
        std::cerr << "Error in " << what << ": " << ec.message() << "\n";
        // Optionally, implement reconnection logic here
    }

    bool is_writing_ = false;
};


// Example usage of the WebSocketClient for high-frequency data
int main() {
  try {
    // Boost.Asio I/O context
    asio::io_context ioc;

    // WebSocket server details
    std::string host = "wss://stream.data.sandbox.alpaca.markets/v1beta3/crypto/us";
    std::string port = "443"; // Use "443" for secure WebSockets (wss)
    std::string target = "/";

    // Create shared pointer of WebSocketClient
    auto client = std::make_shared<WebSocketClient>(ioc, host, port, target);

    // Set up message handler
    client->set_message_handler([](const std::string& msg) {
      // Process incoming messages
      // For high-frequency, ensure this is efficient
      std::cout << "Received: " << msg << std::endl;
    });

    // Run the client
    client->run();

    // Run the I/O context in a separate thread to handle asynchronous operations
    std::thread io_thread([&ioc]() {
      ioc.run();
    });

    // Simulate high-frequency data sending
    // For example, send 100 messages per second
    const int messages_per_second = 100;
    const int total_messages = 1000; // Total messages to send
    const std::chrono::milliseconds interval(1000 / messages_per_second);

    for (int i = 0; i < total_messages; ++i) {
      std::string message = "Message " + std::to_string(i + 1);
      client->send(message);
      std::this_thread::sleep_for(interval);
    }

    // Optionally, wait for some time before closing
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Close the WebSocket connection gracefully
    client->close();

    // Stop the I/O context and join the thread
    ioc.stop();
    if (io_thread.joinable()) {
      io_thread.join();
    }

  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
  }

  return 0;
}