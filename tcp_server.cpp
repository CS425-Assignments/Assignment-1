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
    private:
        int tcp_server_socket;
    public:
        int create_and_bind_socket(int PORT)
        {
            int tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0);
            if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) // creation of TCP socket
            {
                perror("cannot create socket"); 
                return 0; 
            }
            // bind socket to port number
            struct sockaddr_in address;
            address.sin_family = AF_INET; 
            address.sin_addr.s_addr = htonl(INADDR_ANY); 
            address.sin_port = htons(PORT);  
            memset(address.sin_zero, '\0', sizeof address.sin_zero); 
            if (bind(tcp_server_socket,(struct sockaddr *)&address,sizeof(address)) < 0) 
            { 
                perror("bind failed"); 
                exit(EXIT_FAILURE);
            }
            return tcp_server_socket;
        }
};