//
// Copyright (c) 2015 CUHK. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <vector>


void error(const char *msg) {
    perror(msg);
    exit(0);
}

int Connect(struct sockaddr_in td_addr) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    if (connect(sockfd, (struct sockaddr *) &td_addr, sizeof(td_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}


int main(int argc, char *argv[]) {
    struct sockaddr_in td_addr;
    {
        bzero(reinterpret_cast<char *>(&td_addr), sizeof(td_addr));
        td_addr.sin_family = AF_INET;
        struct hostent *server = gethostbyname("localhost");
        if (server == NULL) {
            fprintf(stderr, "ERROR, no such host\n");
            exit(0);
        }
        bcopy(reinterpret_cast<char *>(server->h_addr),
              reinterpret_cast<char *>(&td_addr.sin_addr.s_addr),
              server->h_length);
        td_addr.sin_port = htons(2345);
    }

    while (1) {
        printf("Please enter the message: ");

        char message[256];

        fgets(message, 255, stdin);

        if (strncmp(message, "quit", 4) == 0) {
            std::cout << "Bye-Bye" << std::endl;
            break;
        } else {
            int sockfd = Connect(td_addr);
            // remove Enter key
            message[strlen(message)-1] = '\0';
            std::cout << "Message to be sent: (" << message << ")" << std::endl;
            // include '\0'
            if (write(sockfd, message, strlen(message)+1) < 0)
                error("ERROR writing to socket");
            // char read_buffer[256];
            // bzero(read_buffer, 256);
            // if (read(sockfd, read_buffer, 255) < 0)
            //     error("ERROR reading from socket");

            // printf("(%s)\n", read_buffer);

            close(sockfd);

        }
    }

    return 0;
}
