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

    CTask<float> task;
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
            task.setCameraStatus(CTask<float>::TERMINATE);
            if (t_work.joinable())
                t_work.join();
            task.setCameraStatus(CTask<float>::RUNNING);
            t_work = std::thread(&CTask<float>::Capture, &task, fps);

            comm.Send("OK");
        } else if (action.compare("stop") == 0) {
            task.setCameraStatus(CTask<float>::TERMINATE);
            if (t_work.joinable())
                t_work.join();
            comm.Send("OK");
            break;
        } else {
            std::cerr << "No such command in cd!" << std::endl;
            comm.Send("NO");
        }
    }

    // only unlink after this process ends
    unlink(socket_path.c_str());

    std::cout << "cd is done" << std::endl;
    return 0;
}
