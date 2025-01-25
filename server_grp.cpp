// server-side implementation in C++ for a chat server with private messages and group messaging
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
#include "tcp_server.cpp"
#include "utilities.cpp"

#define BUFFER_SIZE 1024
#define SERVER_PORT 12345
#define MAX_USERS 1000

using namespace std;

int main() {
    TCP_Server HTTP_Server;
    int server_welcome_socket = HTTP_Server.create_and_bind_socket(SERVER_PORT);
    // essential data structures
    unordered_map <string, string> users; // username --> password
    unordered_map <int, string> clients; // socket --> client
    unordered_map <string, unordered_set<int> > groups; // group name --> client sockets
    users = initialise_users();
    


}