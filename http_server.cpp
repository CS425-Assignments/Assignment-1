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
        USER_ALREADY_ONLINE = -7,
        INVALID_COMMAND = -8
    };

    static const int BUFFER_SIZE = 1024;

    bool authenticate_user(int client_socket){
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        string username_prompt = "Enter username: ";
        send(client_socket, username_prompt.c_str(), username_prompt.length(), 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        string username(buffer);

        memset(buffer, 0, BUFFER_SIZE);
        string password_prompt = "Enter password: ";
        send(client_socket, password_prompt.c_str(), password_prompt.length(), 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        string password(buffer);

        sockets_lock.lock();
        if (sockets.find(username) != sockets.end())
        {
            string response = errmsg(USER_ALREADY_ONLINE);
            send(client_socket, response.c_str(), response.length(), 0);
            sockets_lock.unlock();
            close(client_socket);
            return false;
        }
        sockets_lock.unlock();

        users_lock.lock();
        if (users.find(username) == users.end() || users[username] != password)
        {
            string auth_failed = "Authentication failed.";
            send(client_socket, auth_failed.c_str(), auth_failed.length(), 0);
            users_lock.unlock();
            close(client_socket);
            return false;
        }
        users_lock.unlock();

        string welcome_message = "Welcome to the chat server !";
        send(client_socket, welcome_message.c_str(), welcome_message.length(), 0);

        clients_lock.lock();
        clients[client_socket] = username;
        unordered_set<int> recipients;
        for (const auto &[sock, user] : clients)
        {
            if (sock != client_socket)
            {
                recipients.insert(sock);
            }
        }
        clients_lock.unlock();

        sockets_lock.lock();
        sockets[username] = client_socket;
        sockets_lock.unlock();

        string join_message = username + " has joined the chat.";
        send_message(recipients, join_message);

        // Using lambda:
        thread client_thread([this, client_socket]() {
            this->client_handler(client_socket);
        });

        client_thread.detach();
        return true;
    }

    // error handling
    string errmsg(STATUS status){

        string msg = "Error: ";
        switch (status)
        {
        case INVALID_GROUP_NAME:
            msg += "Invalid group name.";
        case INVALID_USER_NAME:
            msg += "Invalid user name.";
        case USER_OFFLINE:
            msg += "User is offline.";
        case USER_NOT_IN_GROUP:
            msg += "User is not in group.";
        case USER_ALREADY_IN_GROUP:
            msg += "User is already in group.";
        case GROUP_EXISTS:
            msg += "Group already exists.";
        case INVALID_COMMAND:
            msg += "Invalid command.";
        case USER_ALREADY_ONLINE:
            msg += "User is already logged in.";
        default:
            msg += "Unknown error.";
        }

        return msg;
    }

    // handlers for commands
    void handle_broadcast(int client_socket, const string& message){
        string client = clients[client_socket];

        string response = "[Broadcast from " + client + "]: " + message;

        unordered_set<int> recipients;
        clients_lock.lock();
        for (const auto &[sock, user] : clients)
        {
            if (sock != client_socket)
            {
                recipients.insert(sock);
            }
        }
        clients_lock.unlock();

        STATUS result = send_message(recipients, response);

        if (result != SUCCESS)
        {
            string response = errmsg(result);
            send(client_socket, response.c_str(), response.length(), 0);
        } // is this fine?
    }

    void handle_private_message(int client_socket, string& request){

		clients_lock.lock();
		if ( clients.find(client_socket) == clients.end()) {
			clients_lock.unlock();
			string response = errmsg(USER_OFFLINE);

			client_locks[client_socket].lock();
            send(client_socket, response.c_str(), response.length(), 0);
			client_locks[client_socket].unlock();
		}

        string client = clients[client_socket];
		clients_lock.unlock();

        string recipient = extract_word(request);
        string message = request;

        string response = "[" + client + "]: " + message;

        unordered_set<int> recipients;
        sockets_lock.lock();
        recipients.insert(sockets[recipient]);
        sockets_lock.unlock();

        STATUS result = send_message(recipients, response);

        if (result != SUCCESS)
        {
            string response = errmsg(result);
			client_locks[client_socket].lock();
            send(client_socket, response.c_str(), response.length(), 0);
			client_locks[client_socket].unlock();
        }
    }

    void handle_create_group(int client_socket, const string& group_name){

        STATUS result = create_group(group_name);

        string response;
        if (result == SUCCESS)
        {
            response = "Group " + group_name + " created.";
            result = join_group(group_name, client_socket);
        }
        else
        {
            response = errmsg(result);
        }

		client_locks[client_socket].lock();
		send(client_socket, response.c_str(), response.length(), 0);
		client_locks[client_socket].unlock();
    }

    void handle_join_group(int client_socket, const string& group_name){
        
        STATUS result = join_group(group_name, client_socket);

        string response;
        if (result == SUCCESS)
        {
            response = "You joined the group " + group_name + ".";
        }
        else
        {
            response = errmsg(result);
        }
		client_locks[client_socket].lock();
		send(client_socket, response.c_str(), response.length(), 0);
		client_locks[client_socket].unlock();

    }

    void handle_leave_group(int client_socket, const string& group_name){
        
        STATUS result = leave_group(group_name, client_socket);

        string response;
        if (result == SUCCESS)
        {
            response = "You left the group " + group_name + ".";
        }
        else
        {
            response = errmsg(result);
        }
		client_locks[client_socket].lock();
		send(client_socket, response.c_str(), response.length(), 0);
		client_locks[client_socket].unlock();

    }

    void handle_group_message(int client_socket, string& request){
        string group_name = extract_word(request);
        string message = request;
        message = "[Group " + group_name + "]: " + message;

        groups_lock.lock();
        if (groups.find(group_name) == groups.end())
        {
            string response = "Group " + group_name + " does not exist.";
            groups_lock.unlock();
            return;
        }

        group_locks[group_name].lock();
        unordered_set<int> recipients = groups[group_name];

		auto it = recipients.find(client_socket);
		recipients.erase(it);

        group_locks[group_name].unlock();
        groups_lock.unlock();

        send_message(recipients, message);
    }

    void handle_client_exit(int client_socket){
        clients_lock.lock();
        string username = clients[client_socket];
        clients.erase(client_socket);
        clients_lock.unlock();

        sockets_lock.lock();
        sockets.erase(username);
        sockets_lock.unlock();

        groups_lock.lock();
        for (auto &[group_name, members] : groups)
        {
            group_locks[group_name].lock();
            members.erase(client_socket);
            group_locks[group_name].unlock();
        }
        groups_lock.unlock();

        close(client_socket);
        cout << "Client " << username << " disconnected." << endl;
    }

    // lowest level network commands
    STATUS send_message(const unordered_set<int> &recv_sockets, const string message){    for (auto &sock : recv_sockets)
        {
            client_locks[sock].lock();
            send(sock, message.c_str(), message.length(), 0);
            client_locks[sock].unlock();
        }

        return SUCCESS;
    }
    STATUS create_group(const string group_name){
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

    STATUS join_group(const string group_name, const int socket){
        groups_lock.lock();

        if (groups.find(group_name) == groups.end())
        {
            cout << "Group " << group_name << " does not exist." << endl;
            groups_lock.unlock();
            return INVALID_GROUP_NAME;
        }

        group_locks[group_name].lock();

        if (groups[group_name].find(socket) != groups[group_name].end())
        {
            cout << "Client " << clients[socket] << " already in group " << group_name << endl;
            group_locks[group_name].unlock();
            groups_lock.unlock();
            return USER_ALREADY_IN_GROUP;
        }

        groups[group_name].insert(socket);
        cout << "Client " << clients[socket] << " joined group " << group_name << endl;
        group_locks[group_name].unlock();

        groups_lock.unlock();

        return SUCCESS;
    }

    STATUS leave_group(const string group_name, const int socket){
        groups_lock.lock();

        if (groups.find(group_name) == groups.end())
        {
            cout << "Group " << group_name << " does not exist." << endl;
            groups_lock.unlock();
            return INVALID_GROUP_NAME;
        }

        group_locks[group_name].lock();

        if (groups[group_name].find(socket) == groups[group_name].end())
        {
            cout << "Client " << clients[socket] << " not in group " << group_name << endl;
            group_locks[group_name].unlock();
            groups_lock.unlock();
            return USER_NOT_IN_GROUP;
        }

        groups[group_name].erase(socket);
        cout << "Client " << clients[socket] << " left group " << group_name << endl;
        group_locks[group_name].unlock();

        groups_lock.unlock();
        return SUCCESS;
    }
    


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
            }
        }
    }
};
