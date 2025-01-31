#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include "http_server.cpp"

#define SERVER_PORT 12345
#define MAX_USERS 1000

// using namespace std;

// unordered_map<string, string> users;              // username --> password
// unordered_map<int, string> clients;               // socket --> client
// unordered_map<string, int> sockets;               // client --> socket
// unordered_map<string, unordered_set<int>> groups; // group name --> client sockets

// mutex users_lock, clients_lock, sockets_lock, groups_lock;
// unordered_map<string, mutex> group_locks;
// unordered_map<int, mutex> client_locks;

// enum STATUS
// {
//     SUCCESS = 0,
//     INVALID_GROUP_NAME = -1,
//     INVALID_USER_NAME = -2,
//     USER_OFFLINE = -3,
//     USER_NOT_IN_GROUP = -4,
//     USER_ALREADY_IN_GROUP = -5,
//     GROUP_EXISTS = -6,
//     INVALID_COMMAND = -7
// };

// STATUS send_message(const unordered_set<int> &recv_sockets, const string message)
// {
//     for (auto &sock : recv_sockets)
//     {
//         client_locks[sock].lock();
//         send(sock, message.c_str(), message.length(), 0);
//         client_locks[sock].unlock();
//     }

//     return SUCCESS;
// }

// STATUS create_group(const string group_name)
// {
//     groups_lock.lock();

//     if (groups.find(group_name) != groups.end())
//     {
//         cout << "Group " << group_name << " already exists." << endl;
//         groups_lock.unlock();

//         return GROUP_EXISTS;
//     }

//     groups[group_name] = {};
//     group_locks[group_name];

//     cout << "Group " << group_name << " created." << endl;

//     groups_lock.unlock();

//     return SUCCESS;
// }

// STATUS join_group(const string group_name, const int socket)
// {
//     groups_lock.lock();

//     if (groups.find(group_name) == groups.end())
//     {
//         cout << "Group " << group_name << " does not exist." << endl;
//         groups_lock.unlock();
//         return INVALID_GROUP_NAME;
//     }

//     group_locks[group_name].lock();

//     if (groups[group_name].find(socket) != groups[group_name].end())
//     {
//         cout << "Client " << clients[socket] << " already in group " << group_name << endl;
//         group_locks[group_name].unlock();
//         groups_lock.unlock();
//         return USER_ALREADY_IN_GROUP;
//     }

//     groups[group_name].insert(socket);
//     cout << "Client " << clients[socket] << " joined group " << group_name << endl;
//     group_locks[group_name].unlock();

//     groups_lock.unlock();

//     return SUCCESS;
// }

// STATUS leave_group(const string group_name, const int socket)
// {
//     groups_lock.lock();

//     if (groups.find(group_name) == groups.end())
//     {
//         cout << "Group " << group_name << " does not exist." << endl;
//         groups_lock.unlock();
//         return INVALID_GROUP_NAME;
//     }

//     group_locks[group_name].lock();

//     if (groups[group_name].find(socket) == groups[group_name].end())
//     {
//         cout << "Client " << clients[socket] << " not in group " << group_name << endl;
//         group_locks[group_name].unlock();
//         groups_lock.unlock();
//         return USER_NOT_IN_GROUP;
//     }

//     groups[group_name].erase(socket);
//     cout << "Client " << clients[socket] << " left group " << group_name << endl;
//     group_locks[group_name].unlock();

//     groups_lock.unlock();
//     return SUCCESS;
// }

// void handle_client_exit(int client_socket)
// {
//     clients_lock.lock();
//     string username = clients[client_socket];
//     clients.erase(client_socket);
//     clients_lock.unlock();

//     sockets_lock.lock();
//     sockets.erase(username);
//     sockets_lock.unlock();

//     groups_lock.lock();
//     for (auto &[group_name, members] : groups)
//     {
//         lock_guard<mutex> group_lock(group_locks[group_name]);
//         members.erase(client_socket);
//     }
//     groups_lock.unlock();

//     close(client_socket);
//     cout << "Client " << username << " disconnected." << endl;
// }

// string extract_word(string &str)
// {
//     string first_word = str.substr(0, str.find(' '));
//     str = str.substr(str.find(' ') + 1);
//     return first_word;
// }

// void client_handler(int client_socket)
// {
//     while (true)
//     {
//         char buffer[BUFFER_SIZE];
//         memset(buffer, 0, BUFFER_SIZE);

//         ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
//         if (bytes_received <= 0)
//         {
//             handle_client_exit(client_socket);
//             break;
//         }

//         string request(buffer);
//         string command = extract_word(request);

//         if (command == "/broadcast")
//         {
//             string message = request;
//             unordered_set<int> recipients;
//             clients_lock.lock();
//             for (const auto &[sock, user] : clients)
//             {
//                 if (sock != client_socket)
//                 {
//                     recipients.insert(sock);
//                 }
//             }
//             clients_lock.unlock();
//             STATUS result = send_message(recipients, "[Broadcast - " + clients[client_socket] + "] " + message);

//             if(result == SUCCESS)
//             {
//                 string response = "Message sent to all online users";
//                 send(client_socket, response.c_str(), response.length(), 0);
//             }
//         }
//         else if (command == "/msg")
//         {
//             string recipient = extract_word(request);
//             string message = request;
//             message = "[" + clients[client_socket] + "]" + message;

//             sockets_lock.lock();
//             if (sockets.find(recipient) != sockets.end())
//             {
//                 int recipient_socket = sockets[recipient];
//                 STATUS result = send_message({recipient_socket}, message);
//                 if(result == SUCCESS)
//                 {
//                     string response = "Message sent to " + recipient;
//                     send(client_socket, response.c_str(), response.length(), 0);
//                 }
//             }
//             else
//             {
//                 string response = "User " + recipient + " not found.";
//                 send(client_socket, response.c_str(), response.length(), 0);
//             }
//             sockets_lock.unlock();
//         }
//         else if (command == "/create_group")
//         {
//             string group_name = request;
//             STATUS result = create_group(group_name);
//             string response = (result == SUCCESS) ? "Group " + group_name + " created." : "Group already exists.";
//             send(client_socket, response.c_str(), response.length(), 0);
//         }
//         else if (command == "/join_group")
//         {
//             string group_name = request;
//             STATUS result = join_group(group_name, client_socket);

//             string response;
//             if (result == SUCCESS)
//                 response = "You joined the group " + group_name + ".";
//             else if (result == INVALID_GROUP_NAME)
//                 response = "Group does not exist.";
//             else
//                 response = "You are already in the group.";

//             send(client_socket, response.c_str(), response.length(), 0);
//         }
//         else if(command == "leave_group")
//         {
//             string group_name = request;

//             STATUS result = leave_group(group_name, client_socket);

//             string response;
//             if (result == SUCCESS)
//                 response = "You left the group " + group_name + ".";
//             else if (result == INVALID_GROUP_NAME)
//                 response = "Group does not exist.";
//             else
//                 response = "You are not in the group.";

//             send(client_socket, response.c_str(), response.length(), 0);
//         }
//         else if (command == "/group_msg")
//         {
//             string group_name = extract_word(request);
//             string message = request;
//             message = "[Group " + group_name + " - " + clients[client_socket] + "]" + message;

//             groups_lock.lock();
//             if (groups.find(group_name) == groups.end())
//             {
//                 string response = "Group " + group_name + " does not exist.";
//                 send(client_socket, response.c_str(), response.length(), 0);
//                 groups_lock.unlock();
//                 continue;
//             }

//             group_locks[group_name].lock();
//             unordered_set<int> recipients = groups[group_name];

//             if(recipients.find(client_socket) == recipients.end())
//             {
//                 string response = "You are not in the group.";
//                 send(client_socket, response.c_str(), response.length(), 0);
//                 group_locks[group_name].unlock();
//                 groups_lock.unlock();
//                 continue;
//             }

//             recipients.erase(client_socket);
//             send_message(recipients, message);
//             group_locks[group_name].unlock();
//             groups_lock.unlock();
//         }

//         else if (command == "/exit")
//         {
//             handle_client_exit(client_socket);
//             break;
//         }
//         else
//         {
//             string response = "Invalid command.";
//             send(client_socket, response.c_str(), response.length(), 0);
//         }
//     }
// }

// bool authenticate_user(int client_socket)
// {
//     char buffer[BUFFER_SIZE];
//     memset(buffer, 0, BUFFER_SIZE);

//     string username_prompt = "Enter username: ";
//     send(client_socket, username_prompt.c_str(), username_prompt.length(), 0);
//     recv(client_socket, buffer, BUFFER_SIZE, 0);
//     string username(buffer);

//     memset(buffer, 0, BUFFER_SIZE);
//     string password_prompt = "Enter password: ";
//     send(client_socket, password_prompt.c_str(), password_prompt.length(), 0);
//     recv(client_socket, buffer, BUFFER_SIZE, 0);
//     string password(buffer);

//     users_lock.lock();
//     if (users.find(username) == users.end() || users[username] != password)
//     {
//         string auth_failed = "Authentication failed.";
//         send(client_socket, auth_failed.c_str(), auth_failed.length(), 0);
//         users_lock.unlock();
//         close(client_socket);
//         return false;
//     }
//     users_lock.unlock();

//     string welcome_message = "Welcome, " + username + "!";
//     send(client_socket, welcome_message.c_str(), welcome_message.length(), 0);

//     clients_lock.lock();
//     clients[client_socket] = username;
//     unordered_set<int> recipients;
//     for (const auto &[sock, user] : clients)
//     {
//         if (sock != client_socket)
//         {
//             recipients.insert(sock);
//         }
//     }
//     clients_lock.unlock();

//     sockets_lock.lock();
//     sockets[username] = client_socket;
//     sockets_lock.unlock();

//     string join_message = username + " has joined the chat.";
//     send_message(recipients, join_message);

//     thread client_thread(client_handler, client_socket);
//     client_thread.detach();
//     return true;
// }

int main()
{
    HTTP_Server ChatServer;
    ChatServer.create_server(SERVER_PORT);
    ChatServer.start_listening(MAX_USERS);
    ChatServer.continuous_accept();
    return 0;
}