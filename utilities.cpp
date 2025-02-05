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

using namespace std;

unordered_map<string, string> initialise_users()
{
    unordered_map<string, string> users;
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
    return users;
}

string extract_word(string& str) {

    if ( str.find(' ') == string::npos ) {
        string first_word = str;
        str = "";
        return first_word;
    }
    string first_word = str.substr(0, str.find(' '));
    str = str.substr(str.find(' ') + 1);
    return first_word;
}

bool invalid_arg(const string& request)
{
    return request.find_first_not_of(' ') == string::npos;
}