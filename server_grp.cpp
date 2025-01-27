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

// essential data structures
unordered_map <string, string> users; // username --> password
unordered_map <int, string> clients; // socket --> client
unordered_map <string, int> sockets; // client --> socket
unordered_map <string, unordered_set<int> > groups; // group name --> client sockets

void send_message(const unordered_set<int>& recv_sockets,const string message)
{
    for (auto& sock : recv_sockets){
        send(sock, message.c_str(), message.length(), 0);
    }
}

void create_group(const string groupname){
    // assumes group does not exist yet

    groups[groupname] = {};

    // create a lock corresponding to this map
}

void join_group(const string groupname, const int socket){

    // lock
    groups[groupname].insert(socket);
    // unlock
}

void leave_group (const string groupname, const int socket){
    // assumes socket is present in group

    // lock
    auto it = groups[groupname].find(socket);
    groups[groupname].erase(it);
    // unlock
}

void client_handler(int client_socket)
{
    // pass
}

int main() {

    users.clear();
    sockets.clear();
    clients.clear();
    groups.clear();
    users = initialise_users();

    TCP_Server HTTP_Server;
    HTTP_Server.create_and_bind_socket(SERVER_PORT);
    int num_users = users.size();
    HTTP_Server.start_listening(num_users);

    // accept incoming connections
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket;

    while (true)
    {
        HTTP_Server.accept_connection(client_socket, address, addrlen);

        // authenticate user
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        string welcome_message = "Enter the user name";
        send(client_socket, welcome_message.c_str(), welcome_message.length(), 0);
        
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) 
        {
            perror("username receive failed");
            exit(EXIT_FAILURE);
        }

        string username = buffer;
        memset(buffer, 0, BUFFER_SIZE);
        string password_message = "Enter the password";
        send(client_socket, password_message.c_str(), password_message.length(), 0);

        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) 
        {
            perror("password receive failed");
            exit(EXIT_FAILURE);
        }

        string password = buffer;

        // authenticate user
        if (users.find(username) == users.end() || users[username] != password) 
        {
            string authentication_failed = "Authentication failed";
            send(client_socket, authentication_failed.c_str(), authentication_failed.length(), 0);
            close(client_socket);
            continue;
        }

        string welcome = "Welcome to the server";
        send(client_socket, welcome.c_str(), welcome.length(), 0);

        // add client to clients
        clients[client_socket] = username;
        sockets[username] = client_socket;

        // create a thread to handle the client
        thread client_thread(client_handler, client_socket);

        // detach the thread
        client_thread.detach();
    }
}