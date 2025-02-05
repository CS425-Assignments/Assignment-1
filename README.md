# Assignment-1

This repository consists of the codebase for the implementation of a live-chat server, as the first Assigment of CS425: Computer Networks, done under Prof. Adithya Vadapalli, department of CSE, IIT Kanpur

<!-- describe elaborately what is the code, how to run and how it works! -->

## Team

| Name                | Roll no. | Email Id                |
| ------------------- | -------- | ----------------------- |
| Aditi Khandelia     | 220061   | aditikh22@iitk.ac.in    |
| Arush Upadhyaya        | 220213   | arushu22@iitk.ac.in   |
| Kushagra Srivastava       | 220573   | skushagra22@iitk.ac.in     |

## Content

- `A1.pdf` - Assigment Description
- `server_grp.cpp` - Driver code for server
- `client_grp.cpp` - Driver code for client
- `tcp_server.cpp` - Basic TCP Server
- `chat_server.cpp` - Chat server built on top of TCP Server
- `utilities.cpp` - Utility functions
- `users.txt` - Users and Passwords
- `Makefile` - Builder file
- `README.md` - Self-explainatory

## Requirements

- `C++20`
- `Make`

## Instructions

### How to run the code?
- Begin with making the project :
```bash
make
```
- Start the server
```bash
./server_grp
```
- If running the chat application as a client, run 
```bash
./client_grp 
```

## Documentation

### Features

A multi-threaded TCP server implementation in C++ supporting group chat functionality. Handles multiple concurrent client connections and broadcasts messages between connected users.

Following are the features provided by the chat server :
- **User Authentication** : The server authenticates the user by checking the username and password in the `users.txt` file.
- **Private Messaging** : The server allows the user to send private messages to any other user.
- **Broadcast Messaging** : The server allows the user to send messages to all the users connected to the server.
- **Group Creation** : The server allows the user to create a group.
- **Group Joining** : The server allows the user to join an existing group.
- **Group Messaging** : The server allows the user to send messages to all the users in a group.
- **Group Leaving** : The server allows the user to leave a group.
- The server is able to handle multiple clients at the same time.

### Design Decisions
Following are the design decisions taken while implementing the chat server :
- **TCP Server** : The server is implemented using TCP sockets.
    * This decision was taken after considering the features required by the chat server : 
        - User Authentication
        - Connection-oriented communication
        - Reliable communication
- **Persistent Connections** : The server maintains a persistent connection with the client. This allows the client to send multiple messages without the need to reconnect.
- **Multithreading** : The server is implemented using multithreading. 
    * The main thread listens for incoming connections.
    * Each client is handled by a separate thread. This allows the server to handle multiple clients at the same time.
    * A user is allowed to communicate with only one connection at a time.
- **Non-pipelined Requests** : The server does not support pipelined requests. This means that the server processes one request at a time from a particular client. However, the server can send multiple responses to the client without waiting for the client to send the next request.


### Implementation Details
Following are the implementation details of the chat server :

- **TCP Server class** : The `tcp_server` class is responsible for creating a TCP server.
    * This allows us to create a server that listens for TCP connections at a specific port (set to 12345).
    * The class also provides the ability to accept multiple welcoming connections simultaneously. <<!-- How? >>
    * Upon getting a connection request, the server creates a connection socket for the client.
- **Chat Server class** : The `chat_server` class is responsible for handling the chat server.
    * As a child class of the `tcp_server` class, it inherits the ability to create a TCP server.
    * The class is responsible for handling the chat server functionalities.
    * The class maintains a list of all the users connected to the server.
    * The class maintains a list of all the groups created by the users.
    * The class maintains a list of all the users in each group.
    * The class maintains a list of the sockets of all the users.
    * The class first binds the welcoming socket to the publicly available port.
    * The class listens for incoming connections.
    * Upon getting a connection request, the server creates a connection socket for the client.
    * The server then authenticates the user by checking the username and password in the `users.txt` file.
    * Upon successful authentication, the server launches a new thread to handle the client (`client_handler`). It also updates the list of users and sockets.
    * `client_handler`: Each thread then listens to the incoming requests for clients, identifies the type of request and calls the appropriate handler for the request.

- **Handler Functions** : The server provides the following handler functions :
    * `handle_private_message` : This function handles the private message sent by the user. 
        * The recipient of the message needs to be online at the time of sending the message.
    * `handle_broadcast_message` : This function handles the broadcast message sent by the user. 
        * This function sends the message to all the users connected to the server.
        * The message is not sent to the user who sent the message.
    * `handle_group_create` : This function handles the group creation request by the user.
        * The group name should be unique.
        * The user who creates the group is automatically added to the group.
    * `handle_group_join` : This function handles the group joining request by the user.
    * The group must exist at the time of this request.
    * `handle_group_message` : This function handles the group message sent by the user.
        * The user should be a part of the group to send a message.
        * The message is sent to all the users in the group other than the sender
    * `handle_group_leave` : This function handles the group leaving request by the user.
        * The user should be a part of the group to leave the group.
        * The user is removed from the group.
- **Network Commands** : Low level functions to carry out network level operations, return the status of the request (either SUCCESS or an error status) :
    * `send_message`: send response to a set of users
    * `create_group` : given a unique group name and if max limit of groups is not reached, creates a group
    * `join_group` : given an existing groupname, adds user's socket to the group's set
    * `leave_group` : given a group that the user is a member of, removes user's socket from the group.

- **Error Handling** : Errors are implemented as an enum `STATUS` where anything other than `SUCCESS` implies an error. If any of the assumptions in a request are violated, request is aborted and error message is sent to the user (`send_error`) alongwith reason in the format: "Error: \<message\>". 

- **Class Diagram** :
![image](./diagram/class%20diagram.png)
- **Code Flow Diagram** :
![image](./diagram/code%20flow%20diagram.png)

### Testing

#### Correctness Testing

Using shell scripting, various scenarios were simulated, that involved mutliple clients logging in the server and interacting with each other using groups/broadcast/private messaging, including the scenario given in [A1.pdf](/A1.pdf)

In scenarios having legitimate requests it was assured that there were no errors while error handling was tested separetly, manually

#### Stress Testing

The server was tested using a `C++` code, that creates multiple threads (using pthreads) and logs in to the server sequentially. 
In order to stress test, dummy users were created with the format `user{i}`. 
- The server was able to handle upto 250 concurrent users. 
- Online users were able to create 2000 groups. 

These constriansts were reached upon while testing the code on our personal computers. CSE servers supported a higher constraint of around a 1000 concurrent users.


### Restrictions in our server

1. Buffer Size : The server supports requests of upto 1024 Bytes in size, however the limit can be changed by setting `BUFFER_SIZE` in [chat_server.cpp](/chat_server.cpp) to the desired value.
2. Maximum Number of Concurrent Users : The server can handle upto 250 concurrent users. This can be changed by setting `MAX_USERS_ONLINE` in [chat_server.cpp](/chat_server.cpp) to the desired value.
3. Maximum Number of Groups : The server can handle upto 2000 groups. This can be changed by setting `MAX_GROUPS` in [chat_server.cpp](/chat_server.cpp) to the desired value.
4. Maximum Number of Users in a Group : The server can handle upto 250 users in a group. This can be changed by setting `MAX_USERS_PER_GROUP` in [chat_server.cpp](/chat_server.cpp) to the desired value.

### Challenges
- **Handling concurrent requests** : One of the main challenges was handling concurrent requests from multiple clients. 
    * We initially considered having the threads accept the requests and putting these requests in a global task list, and have a some worker threads process these tasks. However, this would have limited the concurrency of the requests and introduced a bottleneck.
    * We then decided to have each thread handle the requests for a particular client. This allowed us to handle multiple clients concurrently.
- **Concurrent Access to data structures** : 
    * We had to ensure that the data structures like the list of users, groups, and sockets were accessed in a thread-safe manner.
    * We created mutex locks for each data structure to ensure that only one thread could access the data structure at a time.
    * Mutex locks are available for the global data structures like the list of users, groups, and sockets.
    * Additionally, each handler locks only one data structure at a time to avoid deadlocks.
- **Repetition of basic operations** : 
    * We realised that many operations like sending a message to a user, sending a message to a group, adding a user to a group, etc. were using the same basic functionality of sending a message to a group of recipients.
    * We decided to create a utility function `send_message` that would take the message and the list of recipients and send the message to all the recipients.
    * This allowed us to reuse the code and avoid repetition.
- **Separation of connection interface and logical operations** :
    * We had earlier implemented the logic to accept and parse a request, process it and respond to it in the same function.
    * This made the code difficult to read and maintain.
    * We then decided to separate the connection interface and the logical operations.
    * The connection interface would accept the request, parse it, and call the appropriate processing function.
    * The processing function would process the request, which is sent back to the connection interface to send the response.
    * This allowed us to separate the concerns and make the code more modular. Additionally, error handling became easier.
- **Contradictory requirements from the server** :
    * There can be several instances where the server receives contradictory requests from the client, e.g., a user trying to send a message to a group, whilst another member is leaving said group.
    * The handling of these requests depends on the scheduling of client threads.
    * Each thread the current state of the server and the client, and processes the request accordingly.
    * In case of any inconsistencies, the server sends an appropriate error message to the client.

### Contributions

#### Aditi Khandelia
- Conceptualized the design of the chat server
- Implemented the TCP server class for the chat server
- Implemented the client handler, and sending error messages to the client
- Implemented the lowest level functions for sending and receiving messages
- Correctness testing for the chat server
- Stress testing to check the number of clients in any group
- Documentation of the design decisions and implementation details

#### Arush Upadhyaya
- Conceptualized the design of the chat server
- Implemented the chat server class functions for authentication, accepting connections, and thread creation
- Implemented the lowest level functions for creating and joining groups
- Implemented the Makefile for the project
- Correctness testing for the chat server
- Stress testing to check the number of groups that can be created
- Documentation of the project, team, challenges faced and feedback

#### Kushagra Srivastava
- Conceptualized the design of the chat server
- Implemented the chat server class functions for private messaging, group messaging, and group management
- Implemented the lowest level functions for leaving groups and graceful exit of users
- Implemented the utility functions for sending messages
- Correctness testing for the chat server
- Stress testing to check the number of online connections supported
- Diagrams and documentation of restrictions and stress testing

### Sources 
Following are the sources referred to while implementing the chat server :
- [Kurose and Ross, Computer Networking: A Top-Down Approach](https://gaia.cs.umass.edu/kurose_ross/eighth.php)
- [Medium](https://medium.com/@aryandev512/i-wrote-a-http-server-from-scratch-in-c-0a97e8252371)
- [C++ Reference](https://en.cppreference.com/w/)

### Decalarations

#### Aditi Khandelia
I hereby declare that the work presented in this assignment is solely done by us and has not been copied from any unattributed sources.
#### Arush Upadhyaya
I hereby declare that the work presented in this assignment is solely done by us and has not been copied from any unattributed sources.
#### Kushagra Srivastava
I hereby declare that the work presented in this assignment is solely done by us and has not been copied from any unattributed sources.

### Feedback
Following is our feedback for the assignment :
- **What we liked about the assignment**
    * The assignment was challenging and required us to think about the design of the chat server.
    * The assignment allowed us to implement a real-world application that required us to handle multiple clients concurrently.
    * The assignment was well-structured and provided clear guidelines on the functionalities that needed to be implemented.
- **What we did not like about the assignment**
    * The assignment could have provided more examples of the scenarios that we needed to handle.
    * The assignment could have provided more details on the error handling that was expected.
    * The assignment could have provided more details on the stress testing that was expected.
    * Providing documentation in README is restrictive, latex or word documents could provide more flexibility.

We thank the instructor for providing us with this opportunity to learn and implement a chat server. We look forward to more such assignments in the future.
