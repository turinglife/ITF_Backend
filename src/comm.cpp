//
// ITF Communication Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "comm.hpp"

#include <string>

CComm::~CComm() {
    close(sockfd_);
    unlink(socket_path_.c_str());
}

void CComm::establish(const std::string &socket_name) {
    socket_path_ = socket_name;
    // Create socket
    sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        perror("ERROR socket");
        return;
    }

    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    // strcpy(serv_addr.sun_path, argv[1]);
    snprintf(serv_addr.sun_path, socket_path_.length() + 1, "%s", socket_path_.c_str());

    // Bind socket
    if (bind(sockfd_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR bind");
        return;
    }

    // Start listening
    listen(sockfd_, 5);

    return;
}

void CComm::receive(std::string &action) {
    struct sockaddr_un cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    newsockfd_ = accept(sockfd_, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd_ < 0) {
        perror("ERROR accept");
        return;
    }

    char message[256] = {0};

    if (read(newsockfd_, message, 255) < 0) {
        perror("ERROR read");
        exit(1);
    }

    action = std::string(message);
    close(newsockfd_);
}
