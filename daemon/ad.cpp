//
// ITF Task Daemon, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "task.hpp"
#include "buffer.hpp"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

bool CheckDB(char* task_id) {
    return true;
}

int main(int argc, char *argv[]) {
    // Init socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(2345);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    // Init logger
    // TODO(wangkun): DBI


    // Check the input parameters
    while (1) {
        std::cout << getpid() << ": Wait for client" << std::endl;
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        std::cout << getpid() << ": Found one client" << std::endl;

        pid_t pid = fork();

        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);

            char message[256];

            if (read(newsockfd, message, 255) < 0)
                error("ERROR reading from socket");

            printf("%d: Here is the message: (%s)\n", getpid(), message);

            // Search database and check whether table exists according to task id.
            // TODO(wangkun): This part could be done in CTask

            // Initialize a task object acccording to information retrieved from database.
            CTask task;
            // unsigned int task_id = 0;
            if  (task.LoadTask(message, "ITF.db")) {
                // Capture frames
                task.Capture();
            }

            exit(0);
        } else {
            close(newsockfd);
        }
    }


    close(sockfd);

    return 0;
}
