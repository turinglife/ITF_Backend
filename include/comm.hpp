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
#include "common.hpp"

class CComm {
 public:
  bool Establish(const std::string& socket_path);
  // Receive() will block the process until one client
  // tries to connect it and send a message
  bool Receive(std::string& received_msg);
  // Reply() will send a messag back to the client,
  // and it must be called if the client process uses
  // Send() to send messages
  bool Reply(const std::string& message);
  // Send() will block the process until it receives from
  // the server or the server closes socket
  bool Send(const std::string& message, const std::string& socket_path,
    std::string& received);

 private:
    int server_fd_;
    int client_fd_;
};

#endif  // ITF_COMM_H
