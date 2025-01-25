// Client-side implementation in C++ for a chat server with private messages and group messaging
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

#define BUFFER_SIZE 1024
#define SERVER_PORT 12345

using namespace std;

int main() {
    int server_welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ((server_welcome_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) // creation of TCP socket
    {
        perror("cannot create socket"); 
        return 0; 
    }
    // bind socket to port number
    struct sockaddr_in address;
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons(SERVER_PORT);  
    memset(address.sin_zero, '\0', sizeof address.sin_zero); 
    if (bind(server_welcome_socket,(struct sockaddr *)&address,sizeof(address)) < 0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE);
    }
    // essential data structures
    unordered_map <string, string> users; // username --> password
    unordered_map <int, string> clients; // socket --> client
    unordered_map <string, unordered_set<int> > groups; // group name --> client sockets
    
    ifstream user_file("users.txt");
    int num_users = 0;
    if(user_file.is_open())
    {
        string line;
        while(getline(user_file, line))
        {
            int colon_place = line.find(':');
            string username = line.substr(0, colon_place);
            string password = line.substr(colon_place + 1, line.length() - colon_place - 1);
            users[username] = password;
            num_users++;
        }
    }
    else 
    {
        perror("users.txt open failed"); 
        exit(EXIT_FAILURE);
    }

}