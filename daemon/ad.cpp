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
#include "SQLiteCpp/SQLiteCpp.h"
#include "buffer.hpp"


CTask<float> task;

void work();

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    CComm comm;
    std::string socket_path = "ad_" + task_name;
    comm.establish(socket_path);

    std::thread t_work;

    if (!task.LoadTask(task_name)) {
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

    std::cout << getpid() << ": ad is ready" << std::endl;
    
    while (true) {
        std::string action;
        comm.receive(action);

        if (action.compare("start") == 0) {                                                // START
                task.setFuncStatus(CTask<float>::TERMINATE);
                if (t_work.joinable())
                    t_work.join();
                
                task.setFuncStatus(CTask<float>::RUNNING);
                task.setTaskStatus(CTask<float>::START);
                // Start analyze thread
                t_work = std::thread(work);
        } else if (action.compare("stop") == 0) {                                          // STOP
                task.setFuncStatus(CTask<float>::TERMINATE);
                task.setTaskStatus(CTask<float>::STOP);
                if (t_work.joinable())
                    t_work.join();
                break;
        } else {
            std::cout << "No such command in ad!" << std::endl;
        }
    }

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
        if (!buffer.fetch_src(frame))
            continue;

        vector<float> feature = task.Analyze(frame);
        cv::Mat output(rows, cols, CV_32F, feature.data());

        if (task.getCurrentTaskType() == task.TaskType_t::DENSITY)
        {
            output *= 200.0f;
            // get sum here
        } else if (task.getCurrentTaskType() == task.TaskType_t::SEGMENTATION)
        {
            /* code */
        }
        cv::imshow("ad_result", output);
        cv::waitKey(1);
    }
}
