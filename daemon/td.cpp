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

#include <iostream>
#include <string>
#include <thread>

#include "SQLiteCpp/SQLiteCpp.h"
#include "task.hpp"



static bool on = true;
CTask task;

void show();


int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);
    // Initialize a task object acccording to information retrieved from database.
    
    if  (!task.LoadTask(task_name, "db/ITF.db")) {
        std::cout << "load task fail" << std::endl;
        return -1;
    }

    std::string socket_path = "td_" + task_name;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR socket");
        return -1;
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

    std::cout << getpid() << ": td is ready" << std::endl;
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

        printf("%d: td receive: (%s)\n", getpid(), message);
        close(newsockfd);

        if (strncmp(message, "show", 4) == 0) {
            std::cout << "show()" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();
            on = true;
            t1 = std::thread(show);
        }

        if (strncmp(message, "close", 4) == 0) {
            std::cout << "close()" << std::endl;

            on = false;
        }

        if (strncmp(message, "quit", 4) == 0) {
            std::cout << "quit()" << std::endl;

            on = false;
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

    std::cout << "td is done" << std::endl;
    return 0;
}


void show() {
    // cv::VideoCapture cap(path);
    // if (!cap.isOpened()) {
    //     std::cout << "Cannot Open Video" << std::endl;
    //     return;
    // }
    cv::Mat ini_frame;
    // cap >> ini_frame;
    task.Capture(ini_frame);

    int imgSize = ini_frame.total() * ini_frame.elemSize();

    using boost::interprocess::shared_memory_object;
    using boost::interprocess::open_or_create;
    using boost::interprocess::read_write;

    shared_memory_object shm(open_or_create, task.task_name().c_str(), read_write);
    shm.truncate(imgSize);

    using boost::interprocess::mapped_region;
    mapped_region region(shm, read_write);

    while (on) {
        cv::Mat frame;
        task.Capture(frame);
        if (frame.empty()) {
            break;
        }
        memcpy(region.get_address(), frame.data, imgSize);

        cv::imshow(task.task_name(), frame);
        cv::waitKey(10);
    }

    std::cout << task.task_name() << ": Video is Over" << std::endl;
    if (shared_memory_object::remove("buffer"))
        std::cout << "buffer" << " is released!" << std::endl;
    else
        std::cout << "buffer" << " is NOT released!" << std::endl;
}
