#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h> // For close()
#include <netdb.h>  // For getaddrinfo()
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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

//Throw an SSL error and close the connection
void throwSSLError(std::string ErrMessage, SSL_CTX* ctx, int sfd){
  std::cerr <<  ErrMessage << std::endl;
  ERR_print_errors_fp(stderr);
  close(sfd);
  SSL_CTX_free(ctx);
  cleanup_openssl();
  exit(EXIT_FAILURE);
}


// Establish a TCP connection to the given hostname and port
int create_socket(const std::string& hostname, const std::string& port)
{
    struct addrinfo hints, *res, *p;
    int sockfd, connectfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    int status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &res);
    if (status != 0) std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
    if (status != 0) exit(EXIT_FAILURE);

    // Loop through all the results and connect to the first we can
    for(p = res; p != NULL; p = p->ai_next)
    {
      sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sockfd  == -1)   perror("socket"); //Socket Error
      if (sockfd  == -1)   continue;         //Look for next socket

      connectfd = connect(sockfd, p->ai_addr, p->ai_addrlen);
      if (connectfd == -1) perror("connect");//Connection Error
      if (connectfd == -1) close(sockfd);    //Close the socket

      if((connectfd != -1)and(sockfd  != -1)) break; // If we get here, we have successfully connected
    }

    if (p == NULL) std::cerr << "Failed to connect" << std::endl;
    if (p == NULL) exit(EXIT_FAILURE);

    freeaddrinfo(res); // Free the linked list
    return sockfd;
}

// Perform the HTTPS GET request and return the response
std::string fetchOrderBooks(const std::string path, const std::string hostname, const std::string port)
{
    // Initialize OpenSSL
    initialize_openssl();

    // Create SSL context
    SSL_CTX* ctx = create_context();

    // Create a TCP socket and connect to the server
    int server_fd = create_socket(hostname, port);

    // Create an SSL object
    SSL* ssl = SSL_new(ctx);
    if (!ssl) throwSSLError( "Unable to create SSL structure", ctx, server_fd);
    SSL_set_fd(ssl, server_fd);

    // **Set the SNI hostname**
    bool checkSNI = SSL_set_tlsext_host_name(ssl, hostname.c_str() );
    if(checkSNI != true) SSL_free(ssl);
    if(checkSNI != true) throwSSLError( "Error setting SNI hostname", ctx, server_fd);

    // Perform SSL handshake
    int sslID = SSL_connect(ssl);
    if (sslID <= 0) SSL_free(ssl);
    if (sslID <= 0) throwSSLError( "SSL connection failed", ctx,  server_fd);
 
    // Formulate the HTTP GET request
    std::string request = "GET " + path + " HTTP/1.1\r\n"
                          "Host: " + hostname + "\r\n"
                          "User-Agent: C++ HTTPS Client\r\n"
                          "Accept: */*\r\n"
                          "Connection: close\r\n\r\n";

    // Send the request
    int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
    if (bytes_sent <= 0) SSL_free(ssl);
    if (bytes_sent <= 0) throwSSLError("Failed to send request" , ctx,  server_fd);
 
 
    // Buffer to hold incoming data
    char buffer[BUFFER_SIZE];
    std::string response;

    // Read the response
    while (true)
    {
      int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
      if (bytes_received >  0) response.append(buffer, bytes_received);
      if (bytes_received <= 0) break;
    }

    // Close SSL connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(server_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();


    // Separate headers and body
    std::string headers, body;
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) headers = response.substr(0, pos);
    if (pos != std::string::npos) body = response.substr(pos + 4);
    if (pos == std::string::npos) body = response; // No headers found, return entire response
    return body;
};

int main()
{
//    const std::string path = "/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD,ETH/BTC,ETH/USD";
    const std::string path = "/v1beta3/crypto/us/latest/orderbooks?symbols=BTC/USD";
    const std::string hostname = "data.alpaca.markets";
    const std::string port = "443";

    std::string response = fetchOrderBooks(path, hostname, port);
    if (!response.empty())
    {
      std::cout << "Response from Alpaca API:" << std::endl;
      std::cout << response << std::endl;
    }
    if (response.empty()) std::cerr << "No response received." << std::endl;
    return 0;
}
