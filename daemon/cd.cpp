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
CTask<float> task;

void capture(int fps);

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    // Initialize a task object acccording to information retrieved from database.
    if (!task.LoadTask(task_name, "db/ITF.db")) {
        std::cout << "load task fail" << std::endl;
        return -1;
    }

    if (!task.InitCapture()) {
        std::cout << "init capture fail" << std::endl;
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

        if (action.compare("capture") == 0) {
            std::cout << "start capture" << std::endl;

            on = false;
            if (t1.joinable())
                t1.join();
            on = true;

            t1 = std::thread(capture, 25);
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

    std::cout << "cd is done" << std::endl;
    return 0;
}


void capture(int fps) {
    cv::Mat ini_frame;
    task.Capture(ini_frame);

    int img_width = ini_frame.cols;
    int img_height = ini_frame.rows;
    int img_size = ini_frame.total() * ini_frame.elemSize();
    CBuffer buffer(img_width, img_height, img_size, 50, 30, task.getCurrentTaskName());

    while (on) {
        cv::Mat frame;
        task.Capture(frame);
        if (frame.empty()) {
            break;
        }

        //buffer.put_src(frame);
        if (!buffer.put_src(frame))
            continue;

        // the following two lines are just simulations to display video and might use usleep() instead.
        cv::imshow(task.getCurrentTaskName() + "_frame", frame);
        cv::waitKey(1000 / fps);

        // usleep(1000 * (1000 / fps));
    }
}
