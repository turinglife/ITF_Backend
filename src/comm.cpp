//
// ITF Communication Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "comm.hpp"




void CComm::establish(const std::string &socket_name) {
    //std::string socket_path = "cd_" + task_name;
    // Create socket
    sockfd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        perror("ERROR socket");
        return;
    }
    
    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    // strcpy(serv_addr.sun_path, argv[1]);
    snprintf(serv_addr.sun_path, socket_name.length() + 1, "%s", socket_name.c_str());
    
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
    
    //int length = strlen(message);
    
    action = message;

    printf("%d: cd receive: (%s)\n", getpid(), message);
    close(newsockfd_);

}

void CComm::distroy() {
    close(sockfd_);
}





