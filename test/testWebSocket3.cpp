#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> // For close()
#include <netdb.h>  // For getaddrinfo()
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sstream>
#include <chrono>
#include <ctime>

// Buffer size for reading data
const int BUFFER_SIZE = 4096;

// Initialize OpenSSL
void initialize_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Cleanup OpenSSL
void cleanup_openssl()
{
    EVP_cleanup();
}

// Create an SSL_CTX object
SSL_CTX* create_context()
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = TLS_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Establish a TCP connection to the given hostname and port
int create_socket(const std::string& hostname, const std::string& port)
{
    struct addrinfo hints, *res, *p;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    int status;
    if ((status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Loop through all the results and connect to the first we can
    for(p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("connect");
            continue;
        }

        break; // If we get here, we have successfully connected
    }

    if (p == NULL)
    {
        std::cerr << "Failed to connect" << std::endl;
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res); // Free the linked list

    return sockfd;
}

// Function to get current UTC time in ISO 8601 format
std::string get_current_time_iso8601()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Format: YYYY-MM-DDTHH:MM:SSZ
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%FT%TZ", std::gmtime(&now_c));
    return std::string(buffer);
}

// Function to subtract hours from current time to get start time
std::string get_start_time_iso8601(int hours_back)
{
    auto now = std::chrono::system_clock::now() - std::chrono::hours(hours_back);
    std::time_t start_c = std::chrono::system_clock::to_time_t(now);

    // Format: YYYY-MM-DDTHH:MM:SSZ
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%FT%TZ", std::gmtime(&start_c));
    return std::string(buffer);
}

// Perform the HTTPS GET request and return the response
std::string fetchOrderBooks(const std::string& start_time, const std::string& end_time, const std::string& api_key_id, const std::string& api_secret_key)
{
    const std::string hostname = "data.alpaca.markets";
    const std::string port = "443";

    // Construct the query parameters
    std::ostringstream query_stream;
    query_stream << "/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD,ETH/BTC,ETH/USD";
    query_stream << "&start=" << start_time;
    query_stream << "&end=" << end_time;

    std::string path = query_stream.str();

    // Initialize OpenSSL
    initialize_openssl();

    // Create SSL context
    SSL_CTX* ctx = create_context();

    // Enable certificate verification
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    if (!SSL_CTX_load_verify_locations(ctx, "/etc/ssl/certs/ca-certificates.crt", NULL))
    {
        std::cerr << "Failed to load CA certificates" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    // Create a TCP socket and connect to the server
    int server_fd = create_socket(hostname, port);

    // Create an SSL object
    SSL* ssl = SSL_new(ctx);
    if (!ssl)
    {
        std::cerr << "Unable to create SSL structure" << std::endl;
        ERR_print_errors_fp(stderr);
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    SSL_set_fd(ssl, server_fd);

    // Set the SNI hostname
    if (!SSL_set_tlsext_host_name(ssl, hostname.c_str()))
    {
        std::cerr << "Error setting SNI hostname" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    // Perform SSL handshake
    if (SSL_connect(ssl) <= 0)
    {
        std::cerr << "SSL connection failed" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    // Formulate the HTTP GET request with headers
    std::ostringstream request_stream;
    request_stream << "GET " << path << " HTTP/1.1\r\n"
                   << "Host: " << hostname << "\r\n"
                   << "User-Agent: MyCryptoApp/1.0 (C++ HTTPS Client)\r\n"
                   << "Accept: application/json\r\n"
                   << "APCA-API-KEY-ID: " << api_key_id << "\r\n"
                   << "APCA-API-SECRET-KEY: " << api_secret_key << "\r\n"
                   << "Connection: close\r\n\r\n";

    std::string request = request_stream.str();

    // Send the request
    int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
    if (bytes_sent <= 0)
    {
        std::cerr << "Failed to send request" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    // Buffer to hold incoming data
    char buffer[BUFFER_SIZE];
    std::string response;

    // Read the response
    while (true)
    {
        int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes_received > 0)
        {
            response.append(buffer, bytes_received);
        }
        else
        {
            break;
        }
    }

    // Close SSL connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    // Separate headers and body
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        std::string headers = response.substr(0, pos);
        std::string body = response.substr(pos + 4);
        // Optionally, you can process headers if needed
        return body;
    }
    else
    {
        // No headers found, return entire response
        return response;
    }
}

int main()
{
    // Define the fixed points in time
    // Example: Fetch data from 24 hours ago to now
    // Adjust the 'hours_back' as needed
    int hours_back = 24;

    std::string end_time = get_current_time_iso8601(); // Current UTC time
    std::string start_time = get_start_time_iso8601(hours_back); // Start time

    // API Credentials (Replace with your actual credentials)
    std::string api_key_id = "your_api_key_id";
    std::string api_secret_key = "your_secret_key";

    // Fetch the order books
    std::string response = fetchOrderBooks(start_time, end_time, api_key_id, api_secret_key);

    if (!response.empty())
    {
        std::cout << "Response from Alpaca API:" << std::endl;
        std::cout << response << std::endl;
    }
    else
    {
        std::cerr << "No response received." << std::endl;
    }

    return 0;
}
