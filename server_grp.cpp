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
#include "chat_server.cpp"

#define SERVER_PORT 12345
#define MAX_USERS 250

int main()
{
    Chat_Server ChatServer;
    ChatServer.create_server(SERVER_PORT);
    ChatServer.start_listening(MAX_USERS);
    ChatServer.continuous_accept();
    return 0;
}