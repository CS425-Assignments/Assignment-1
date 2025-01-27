// tcp-server object implementation
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

class TCP_Server
{
public:
    int tcp_server_socket;

    TCP_Server() : tcp_server_socket(-1) {}

    ~TCP_Server()
    {
        if (tcp_server_socket >= 0)
        {
            close(tcp_server_socket);
        }
    }

    void create_and_bind_socket(int PORT)
    {
        tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0);
        
        if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Cannot create socket.");
        }
        // Bind socket to port number
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(PORT);
        memset(address.sin_zero, '\0', sizeof address.sin_zero);
        if (bind(tcp_server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("Bind failed.");
            exit(EXIT_FAILURE);
        }
        std::cout << "Server running on port " << PORT << std::endl;
    }

    void start_listening(int num_users)
    {
        if (::listen(tcp_server_socket, num_users) < 0)
        {
            perror("Server listen failed.");
            exit(EXIT_FAILURE);
        }
    }

    void accept_connection(int &client_socket, struct sockaddr_in &address, int &addrlen)
    {
        if ((client_socket = accept(tcp_server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Server accept failed.");
            exit(EXIT_FAILURE);
        }
    }
};