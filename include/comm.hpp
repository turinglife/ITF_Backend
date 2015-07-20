//
// ITF Communication Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_COMM_H
#define ITF_COMM_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <string>

class Server {
 public:
    bool Establish(const std::string& socket_path);

    // Server::Receive() will block the process until one client tries to connect it and send a message
    bool Receive(std::string& received_msg);

    // Server::Send() will send a messag back to the client, 
    // and it must be called if the client process uses Clent::SendTo() to send messages
    bool Send(const std::string& message);

 private:
    int server_fd_ = 0;
    int client_fd_ = 0;
};

class Client {
 public:
    // Client::SendTo() will block the process until it receives from the server or the server closes socket
    static bool SendTo(const std::string& message, const std::string& socket_path, std::string& received);
};


#endif  // ITF_COMM_H
