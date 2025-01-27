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

unordered_map<string, string> users;              // username --> password
unordered_map<int, string> clients;               // socket --> client
unordered_map<string, int> sockets;               // client --> socket
unordered_map<string, unordered_set<int>> groups; // group name --> client sockets

mutex users_lock, clients_lock, sockets_lock, groups_lock;
unordered_map <string, mutex> group_locks;
unordered_map <int, mutex> client_locks;

enum STATUS
{
    SUCCESS = 0,
    INVALID_GROUP_NAME = -1,
    INVALID_USER_NAME = -2,
    USER_OFFLINE = -3,
    USER_NOT_IN_GROUP = -4,
    USER_ALREADY_IN_GROUP = -5,
    GROUP_EXISTS = -6,
    INVALID_COMMAND = -7
};

void send_message(const unordered_set<int>& recv_sockets,const string message)
{
    for (auto &sock : recv_sockets)
    {
        send(sock, message.c_str(), message.length(), 0);
    }
}

STATUS create_group(const string group_name)
{
    groups_lock.lock();

    if (groups.find(group_name) != groups.end()) 
    {
        cout << "Group " << group_name << " already exists." << endl;
        groups_lock.unlock();

        return GROUP_EXISTS;
    }

    groups[group_name] = {};
    group_locks[group_name]; 

    cout << "Group " << group_name << " created." << endl;

    groups_lock.unlock();

    return SUCCESS;
}

STATUS join_group(const string group_name, const int socket)
{
    groups_lock.lock();

    if (groups.find(group_name) == groups.end())
    {
        cout << "Group " << group_name << " does not exist." << endl;
        groups_lock.unlock();
        return INVALID_GROUP_NAME;
    }

    group_locks[group_name].lock();

    if(groups[group_name].find(socket) != groups[group_name].end())
    {
        cout << "Client " << clients[socket] << " already in group." << group_name << endl;
        group_locks[group_name].unlock();
        groups_lock.unlock();
        return USER_ALREADY_IN_GROUP;
    }

    groups[group_name].insert(socket);
    cout << "Client " << clients[socket] << " joined group." << group_name << endl;
    group_locks[group_name].unlock();

    groups_lock.unlock();

    return SUCCESS;
}

STATUS leave_group(const string group_name, const int socket)
{
    groups_lock.lock();

    if (groups.find(group_name) == groups.end())
    {
        cout << "Group " << group_name << " does not exist." << endl;
        groups_lock.unlock();
        return INVALID_GROUP_NAME;
    }

    group_locks[group_name].lock();

    if(groups[group_name].find(socket) == groups[group_name].end())
    {
        cout << "Client " << clients[socket] << " not in group." << group_name << endl;
        group_locks[group_name].unlock();
        groups_lock.unlock();
        return USER_NOT_IN_GROUP;
    }

    groups[group_name].erase(socket);   
    cout << "Client " << clients[socket] << " left group." << group_name << endl;
    group_locks[group_name].unlock();

    groups_lock.unlock();
    return SUCCESS;
}

void client_handler(int client_socket)
{
    // pass
}

bool authenticate_user(int client_socket)
{
    // Authenticate user
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    string username_message = "Enter the user name : ";
    send(client_socket, username_message.c_str(), username_message.length(), 0);

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        perror("Username receive failed.");
        return false;
    }
    string username = buffer;

    memset(buffer, 0, BUFFER_SIZE);
    string password_message = "Enter the password : ";
    send(client_socket, password_message.c_str(), password_message.length(), 0);

    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0)
    {
        perror("Password receive failed");
        return false;
    }
    string password = buffer;

    if (users.find(username) == users.end() || users[username] != password)
    {
        string authentication_failed = "Authentication failed";
        send(client_socket, authentication_failed.c_str(), authentication_failed.length(), 0);
        close(client_socket);
        return false;
    }

    string welcome = "Welcome to the chat server!";
    send(client_socket, welcome.c_str(), welcome.length(), 0);

    // Update the mappings
    clients[client_socket] = username;
    sockets[username] = client_socket;

    // Create a thread to handle the client
    thread client_thread(client_handler, client_socket);

    client_thread.detach();
    return true;
}

int main()
{
    users.clear();
    sockets.clear();
    clients.clear();
    groups.clear();
    users = initialise_users();

    TCP_Server HTTP_Server;
    HTTP_Server.create_and_bind_socket(SERVER_PORT);
    int num_users = users.size();
    HTTP_Server.start_listening(num_users);

    // Accept incoming connections
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket;

    while (true)
    {
        HTTP_Server.accept_connection(client_socket, address, addrlen);

        if(!authenticate_user(client_socket))
        {
            close(client_socket);
            continue;
        }

        thread client_thread(client_handler, client_socket);
        client_thread.detach();
    }
}