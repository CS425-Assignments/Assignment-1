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
#include "tcp_server.cpp"
#include "utilities.cpp"

class HTTP_Server : public TCP_Server
{
    private :

    enum STATUS {
        SUCCESS = 0,
        INVALID_GROUP_NAME = -1,
        INVALID_USER_NAME = -2,
        USER_OFFLINE = -3,
        USER_NOT_IN_GROUP = -4,
        USER_ALREADY_IN_GROUP = -5,
        GROUP_EXISTS = -6,
        INVALID_COMMAND = -7
    };

    static const int BUFFER_SIZE = 1024;

    bool authenticate_user(int client_socket); // to be filled


    // handlers for commands, to be shifted
    void handle_broadcast(int client_socket, const string& message);
    void handle_private_message(int client_socket, string& request);
    void handle_create_group(int client_socket, const string& group_name);
    void handle_join_group(int client_socket, const string& group_name);
    void handle_leave_group(int client_socket, const string& group_name);
    void handle_group_message(int client_socket, const string& request);
    void handle_client_exit(int client_socket);

    // lowest level network commands
    STATUS send_message(const unordered_set<int> &recv_sockets, const string message);
    STATUS create_group(const string group_name);
    STATUS join_group(const string group_name, const int socket);
    STATUS leave_group(const string group_name, const int socket);
    


    public :
    unordered_map<string, string> users;              // username --> password
    unordered_map<int, string> clients;               // socket --> client
    unordered_map<string, int> sockets;               // client --> socket
    unordered_map<string, unordered_set<int>> groups; // group name --> client sockets

    mutex users_lock, clients_lock, sockets_lock, groups_lock;
    unordered_map<string, mutex> group_locks;
    unordered_map<int, mutex> client_locks;

    void create_server(int PORT)
    {
        users = initialise_users();
        create_and_bind_socket(PORT);
    }
    void start_listening(int num_users)
    {
        start_listening(num_users);
    }
    void continuous_accept()
    {
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int client_socket;

        while (true)
        {
            TCP_Server::accept_connection(client_socket, address, addrlen);
            if(!authenticate_user(client_socket))
            {
                close(client_socket);
                continue;
            }
        }
    }

    void client_handler(int client_socket) {
        while (true) {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received <= 0) {
                handle_client_exit(client_socket);
                break;
            }

            string request(buffer);
            string command = extract_word(request);

            if (command == "/broadcast") {
                handle_broadcast(client_socket, request);
            }
            else if (command == "/msg") {
                handle_private_message(client_socket, request);
            }
            else if (command == "/create_group") {
                handle_create_group(client_socket, request);
            }
            else if (command == "/join_group") {
                handle_join_group(client_socket, request);
            }
            else if (command == "/leave_group") {
                handle_leave_group(client_socket, request);
            }
            else if (command == "/group_msg") {
                handle_group_message(client_socket, request);
            }
            else if (command == "/exit") {
                handle_client_exit(client_socket);
                break;
            }
            else {
                string response = "Invalid command.";
                send(client_socket, response.c_str(), response.length(), 0);
            }
        }
    }
};