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

    //Server comm;
    CComm server;
    std::string socket_path = "cd_" + task_name;
    if (!server.Establish(socket_path)) {
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
        server.Receive(action);

        if (action.compare("start") == 0) {
            task.setCameraStatus(CTask<float>::TERMINATE);
            if (t_work.joinable())
                t_work.join();
            task.setCameraStatus(CTask<float>::RUNNING);
            t_work = std::thread(&CTask<float>::Capture, &task, fps);

            server.Reply("OK");
        } else if (action.compare("stop") == 0) {
            task.setCameraStatus(CTask<float>::TERMINATE);
            if (t_work.joinable())
                t_work.join();
            server.Reply("OK");
            break;
        } else {
            std::cerr << "No such command in cd!" << std::endl;
            server.Reply("NO");
        }
    }
<<<<<<< HEAD
    
    //task.~CTask();
=======
>>>>>>> d17d2db9e3cb9c0f114db78d45db1b00ee5adab6

    // only unlink after this process ends
    unlink(socket_path.c_str());

    std::cout << "cd is done" << std::endl;
    return 0;
}
<<<<<<< HEAD

void tCapture(int fps) {
    int rows = task.getCurrentFrameHeight();
    int cols = task.getCurrentFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();

    CBuffer buffer(cols, rows, imgSize, 50, 30, task.getCurrentTaskName());

    while (on) {
        cv::Mat frame;
        
        // Capture the frames from videos or cameras.
        task.Capture(frame);
        if (frame.empty()) {
            break;
        }

        // Put the captured frames to the buffer.
        if (!buffer.put_src(frame))
            continue;
        
        // Show frames when getting frames from buffer successfully.
        if (buffer.fetch_frame(frame)) {
            cv::imshow(task.getCurrentTaskName() + "_frame", frame);
            cv::waitKey(1000 / fps);
        }
        
    }
}
=======
>>>>>>> d17d2db9e3cb9c0f114db78d45db1b00ee5adab6
