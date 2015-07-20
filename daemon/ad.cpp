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

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "task.hpp"
#include "comm.hpp"
#include "buffer.hpp"


static CTask<float> task;

void work();

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    //Server comm;
    CComm server;
    std::string socket_path = "ad_" + task_name;
    if (!server.Establish(socket_path)) {
        std::cerr << "Fail to establish connection" << std::endl;
        return -1;
    }

    if (!task.LoadTask(task_name)) {
        unlink(socket_path.c_str());
        std::cerr << "load task fail" << std::endl;
        std::cerr << "ad exit" << std::endl;
        return -1;
    }
    // Init analyzer
    if (!task.InitAnalyzer()) {
        std::cerr << "init analyzer fail" << std::endl;
        std::cerr << "ad exit" << std::endl;
        return -1;
    }

    std::thread t_work;

    std::cout << "ad is ready" << std::endl;

    while (true) {
        std::string action;
        server.Receive(action);

        if (action.compare("start") == 0) {  // START
                task.setFuncStatus(CTask<float>::TERMINATE);
                if (t_work.joinable())
                    t_work.join();

                task.setFuncStatus(CTask<float>::RUNNING);
                task.setTaskStatus(CTask<float>::START);
                // Start analyze thread
                t_work = std::thread(work);
                server.Reply("OK");
        } else if (action.compare("stop") == 0) {  // STOP
                task.setFuncStatus(CTask<float>::TERMINATE);
                task.setTaskStatus(CTask<float>::STOP);
                if (t_work.joinable())
                    t_work.join();
                server.Reply("OK");
                break;
        } else {
            std::cerr << "No such command in ad!" << std::endl;
            server.Reply("NO");
        }
    }
    
    
    // only unlink after this process ends
    unlink(socket_path.c_str());

    std::cout << "ad is done" << std::endl;
    return 0;
}

void work() {
    int rows = task.getCurrentFrameHeight();
    int cols = task.getCurrentFrameWidth();
    cv::Mat frame(rows, cols, CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();

    CBuffer buffer(task.getCurrentTaskName());
    buffer.init(imgSize);

    while (task.getFuncStatus()) {
        if (!buffer.fetch_src(frame)) {
            std::cerr << "ad: No Available Frame for " << task.getCurrentTaskName() << std::endl;
            sleep(1);  // reduce useless while loop
            continue;
        }

        vector<float> feature = task.Analyze(frame);
        cv::Mat output(rows, cols, CV_32F, feature.data());

        if (task.getCurrentTaskType() == task.TaskType_t::DENSITY) {
            output *= 256.0f;
            // get sum here
        } else if (task.getCurrentTaskType() == task.TaskType_t::SEGMENTATION) {
            /* code */
        }
        cv::imshow("ad_result", output);
        cv::waitKey(1);
    }
}
