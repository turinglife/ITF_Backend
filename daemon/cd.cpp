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
    if (!task.LoadTask(task_name)) {
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

    std::thread t_work;

    std::cout << getpid() << ": cd is ready" << std::endl;
    while (true) {
        std::string action;
        comm.receive(action);

        if (action.compare("start") == 0) {
            on = false;
            if (t_work.joinable())
                t_work.join();
            on = true;

            t_work = std::thread(capture, 25);
        } else if (action.compare("stop") == 0) {
            on = false;
            if (t_work.joinable())
                t_work.join();
            break;
        } else {
            std::cerr << "No such command in cd!" << std::endl;
        }
    }
    task.~CTask();
    std::cout << "cd is done" << std::endl;
    return 0;
}

void capture(int fps) {
    int rows = task.getCurrentFrameHeight();
    int cols = task.getCurrentFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();
    
    CBuffer buffer(cols, rows, imgSize, 50, 30, task.getCurrentTaskName());

    while (on) {
        cv::Mat frame;
        task.Capture(frame);
        if (frame.empty()) {
            break;
        }

        // the following two lines are just simulations to display video and might use usleep() instead.
        // usleep(1000 * (1000 / fps));
        cv::imshow(task.getCurrentTaskName() + "_frame", frame);
        cv::waitKey(1000 / fps);

        if (!buffer.put_src(frame))
            continue;
    }
}
