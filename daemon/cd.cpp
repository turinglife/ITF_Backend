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

#include "task.hpp"
#include "buffer.hpp"


static bool on = true;
static CTask<float> task;

void tCapture(int fps);

int main(int argc, char* argv[]) {
    // Initialize Google's logging library.
     ::google::InitGoogleLogging(argv[0]);
     // Print output to stderr (while still logging).
    FLAGS_alsologtostderr = 1;

    std::string task_name(argv[1]);
    CHECK(!task_name.empty()) << "task_name cannot be empty";

    Server comm;
    std::string socket_path = "cd_" + task_name;
    if (!comm.Establish(socket_path)) {
        std::cerr << "Fail to establish connection" << std::endl;
        std::cerr << "cd exit" << std::endl;
        return -1;
    }

    // Initialize a task object
    if (!task.LoadTask(task_name)) {
        unlink(socket_path.c_str());
        std::cerr << "load task fail" << std::endl;
        std::cerr << "cd exit" << std::endl;
        return -1;
    }

    if (!task.InitCapture()) {
        std::cerr << "init capture fail" << std::endl;
        std::cerr << "cd exit" << std::endl;
        return -1;
    } else {
        std::cout << "Camera is initialized" << std::endl;
    }

    int fps = 30;  // we may consider to move fps to CTask;
    std::thread t_work;

    LOG(INFO) << "cd is ready";

    while (true) {
        std::string action;
        comm.Receive(action);

        if (action.compare("start") == 0) {
            on = false;
            if (t_work.joinable())
                t_work.join();
            on = true;

            t_work = std::thread(tCapture, fps);

            comm.Send("OK");
        } else if (action.compare("stop") == 0) {
            on = false;
            if (t_work.joinable())
                t_work.join();
            comm.Send("OK");
            break;
        } else {
            std::cerr << "No such command in cd!" << std::endl;
            comm.Send("NO");
        }
    }
    task.~CTask();

    // only unlink after this process ends
    unlink(socket_path.c_str());

    std::cout << "cd is done" << std::endl;
    return 0;
}

void tCapture(int fps) {
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

        cv::imshow(task.getCurrentTaskName() + "_frame", frame);
        cv::waitKey(1000 / fps);
        // usleep(1000 * (1000 / fps));

        if (!buffer.put_src(frame))
            continue;
    }
}
