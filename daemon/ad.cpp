//
// Copyright 2015 CUHK
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <opencv2/highgui/highgui.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "task.hpp"
#include "SQLiteCpp/SQLiteCpp.h"


void density(std::string path);
void segmentation(std::string path);

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    CTask task;
    if  (!task.LoadTask(task_name, "db/ITF.db")) {
        std::cout << "load task fail" << std::endl;
        return -1;
    }

    std::string socket_path = "ad_" + task_name;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR socket");
        exit(1);
    }

    struct sockaddr_un serv_addr;
    serv_addr.sun_family = AF_UNIX;
    // strcpy(serv_addr.sun_path, argv[1]);
    snprintf(serv_addr.sun_path, socket_path.length() + 1, "%s", socket_path.c_str());

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR bind");
        return -1;
    }

    listen(sockfd, 5);

    std::thread t1;

    std::cout << getpid() << ": ad is ready" << std::endl;
    while (true) {
        struct sockaddr_un cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR accept");
            return -1;
        }

        char message[256];

        if (read(newsockfd, message, 255) < 0) {
            perror("ERROR read");
            exit(1);
        }

        printf("%d: ad receive: (%s)\n", getpid(), message);
        close(newsockfd);

        if (strncmp(message, "density", 7) == 0) {
            std::cout << "density()" << std::endl;

            task.on = false;
            if (t1.joinable())
                t1.join();
            task.on = true;
            t1 = task.Analyze(CTask::FunType_t::COUNT, task_name);
        }

        if (strncmp(message, "segmentation", 12) == 0) {
            std::cout << "segmentation()" << std::endl;

            task.on = false;
            if (t1.joinable())
                t1.join();
            task.on = true;
            t1 = task.Analyze(CTask::FunType_t::SEGMENT, task_name);
        }

        if (strncmp(message, "close", 4) == 0) {
            std::cout << "close()" << std::endl;

            task.on = false;
        }

        if (strncmp(message, "quit", 4) == 0) {
            std::cout << "quit()" << std::endl;

            task.on = false;
            task.on = false;
            if (t1.joinable())
                t1.join();

            break;
        }
    }

    close(sockfd);

    if (unlink(socket_path.c_str()) == -1) {
        std::cout << "ERROR unlink: " << socket_path << std::endl;
        return -1;
    }

    std::cout << "ad is done" << std::endl;
    return 0;
}
