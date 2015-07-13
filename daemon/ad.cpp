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

void analyze();


int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    if (!task.LoadTask(task_name, "ITF")) {
        std::cout << "load task fail" << std::endl;
        return -1;
    }

    if (!task.InitAnalyzer()) {
        std::cout << "init analyzer fail" << std::endl;
        return -1;
    }

    std::string socket_path = "ad_" + task_name;

    CComm comm;
    comm.establish(socket_path);

    std::thread t1;

    std::cout << getpid() << ": ad is ready" << std::endl;
    while (true) {
        std::string action;
        comm.receive(action);

        if (action.compare("analyze") == 0) {
            std::cout << "start analyze" << std::endl;

            task.on = false;
            if (t1.joinable())
                t1.join();
            task.on = true;
            t1 = std::thread(analyze);
        }

        if (action.compare("close") == 0) {
            std::cout << "close()" << std::endl;

            task.on = false;
        }

        if (action.compare("quit") == 0) {
            std::cout << "quit()" << std::endl;

            task.on = false;
            task.on = false;
            if (t1.joinable())
                t1.join();

            break;
        }
    }

    std::cout << "ad is done" << std::endl;
    return 0;
}

void analyze() {
    cv::Mat frame(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();

    std::cout << "analyze" << std::endl;

    CBuffer buffer(task.getCurrentTaskName());
    std::cout << "init buffer " << endl;
    buffer.init(imgSize);
    std::cout << "init buffer done" << endl;
    itf::Util util;
    while (task.on) {
        if (!buffer.fetch_src(frame))
            continue;
        vector<float> feature = task.Analyze(frame);

        cv::Mat output(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_32F, feature.data());

        /*
            Note:
                For different kinds of analyze, there are different post-processing

                    1. density: needs to call Util::GenerateHeatMap(cv::Mat pMap) and get sum;
                    2. segment: needs to set threshhold on probaility map;
                    3. stationary: no post-processing
                    4. group: some strange post-processing which I cannot figure out.
        */

        // 1. Density:
        cv::Mat pMap = cv::Mat::ones(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_32F);
        cv::Mat heat = util.GenerateHeatMap(output, pMap);
        cv::imshow("heat", heat);
        cv::waitKey(1);

        // 2. Segment:
        // cv::Mat foreground = output > 0.5;
        // cv::imshow("foreground", foreground);
        // cv::waitKey(1);
    }
}
