//
// ITF Communication Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "comm.hpp"

bool CComm::Establish(const std::string& socket_path) {
  // Create server socket
  server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    perror("ERROR socket");
    return false;
  }
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  snprintf(server_addr.sun_path, socket_path.length() + 1,
    "%s", socket_path.c_str());
  // Bind socket
  if (bind(server_fd_, (struct sockaddr *) &server_addr,
      sizeof(server_addr)) < 0) {
    perror("ERROR bind");
    return false;
  }
  // Start listening
  listen(server_fd_, 5);
  return true;
}

bool CComm::Receive(std::string& received_msg) {
  struct sockaddr_un client_addr;
  socklen_t clilen = sizeof(client_addr);
  client_fd_ = accept(server_fd_, (struct sockaddr *) &client_addr, &clilen);
  if (client_fd_ < 0) {
    perror("ERROR accept");
    return false;
  }
  char message[256] = {0};
  if (read(client_fd_, message, 255) < 0) {
    perror("ERROR read");
    return false;
  } else {
    received_msg = std::string(message);
    return true;
  }
}

bool CComm::Reply(const std::string& message) {
  if (write(client_fd_, message.c_str(), message.length() + 1) < 0) {
    perror("ERROR write");
    return false;
  } else {
    close(client_fd_);
    return true;
  }
}

bool CComm::Send(const std::string& message, const std::string& socket_path,
    std::string& received) {
  // create a new client
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR socket");
    return false;
  }
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  snprintf(server_addr.sun_path, socket_path.length() + 1,
    "%s", socket_path.c_str());
  if (connect(sockfd, (struct sockaddr *) &server_addr,
      sizeof(server_addr)) < 0) {
    perror("ERROR connect");
    return false;
  }
  if (write(sockfd, message.c_str(), message.length() + 1) < 0) {
    perror("ERROR write");
    return false;
  }
  char tmp_msg[256] = {0};
  if (read(sockfd, tmp_msg, 255) <= 0) {
    perror("ERROR read");
  } else {
    received = std::string(tmp_msg);
  }
  close(sockfd);
  return true;
}
