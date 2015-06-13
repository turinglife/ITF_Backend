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
#include "comm.hpp"

#include "SQLiteCpp/SQLiteCpp.h"
#include "task.hpp"
#include "buffer.hpp"


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

    // Establish a new communication connection with other processes.
    std::string socket_name = "cd_" + task_name;

    CComm comm;
    comm.establish(socket_name);

    std::thread t1;

    std::cout << getpid() << ": cd is ready" << std::endl;
    while (true) {
        std::string action;
        comm.receive(action);

        if (action.compare("show") == 0) {
            std::cout << "show()" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();
            on = true;
            t1 = std::thread(show);
        }

        if (action.compare("close") == 0) {
            std::cout << "close()" << std::endl;

            on = false;
        }

        if (action.compare("quit") == 0) {
            std::cout << "quit()" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();

            break;
        }
    }

    comm.distroy();

    if (unlink(socket_name.c_str()) == -1) {
        std::cout << "ERROR unlink: " << socket_name << std::endl;
        return -1;
    }

    std::cout << "cd is done" << std::endl;

    return 0;
}


void show() {
    cv::Mat ini_frame;
    task.Capture(ini_frame);

    int img_size = ini_frame.total() * ini_frame.elemSize();

    CBuffer buffer(img_size, task.task_name());

    while (on) {
        cv::Mat frame;
        task.Capture(frame);
        if (frame.empty()) {
            break;
        }

        buffer.put(frame);

        cv::imshow(task.task_name() + "_frame", frame);
        cv::waitKey(10);
    }

    std::cout << task.task_name() << ": Video is Over" << std::endl;
}
