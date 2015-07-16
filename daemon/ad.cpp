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

    //if (!task.LoadTask(task_name, "ITF")) {
    //    std::cout << "load task fail" << std::endl;
    //    return -1;
    //}

    //if (!task.InitAnalyzer()) {
    //    std::cout << "init analyzer fail" << std::endl;
    //    return -1;
    //}

    std::string socket_path = "ad_" + task_name;

    CComm comm;
    comm.establish(socket_path);

    std::thread t1;

    std::cout << getpid() << ": ad is ready" << std::endl;
    while (true) {
        std::string action;
        comm.receive(action);

        if (action.compare("start") == 0) {                                                // START
            std::cout << "start analyze thread" << std::endl;

            if (!task.LoadTask(task_name, "ITF")) {
                std::cout << "load task fail" << std::endl;

                return -1;
            }

            // Check the current task status from database.
            CTask<float>::TaskStatus_t TaskStatus = static_cast<CTask<float>::TaskStatus_t>(task.getTaskStatus());
            // Check the current function status
            CTask<float>::FuncStatus_t FuncStatus = static_cast<CTask<float>::FuncStatus_t>(task.getFuncStatus());

            if (TaskStatus == CTask<float>::START && FuncStatus == CTask<float>::TERMINAL) {

                if (t1.joinable())
                    t1.join();

                // Start analyze thread
                t1 = std::thread(analyze);

                // Set the current task status as RUNNING
                task.setFuncStatus(CTask<float>::RUNNING);

            } else if (TaskStatus == CTask<float>::START && FuncStatus == CTask<float>::RUNNING) {

                // Nothing to do, but recording this abnormal action.

            } else if (TaskStatus == CTask<float>::STOP) {

                if (t1.joinable())
                    t1.join();

                // Start analyze thread
                t1 = std::thread(analyze);

                // Set the current task status as RUNNING
                task.setFuncStatus(CTask<float>::RUNNING);
                task.setTaskStatus(CTask<float>::START);

                // Update task status(CTask::START) to database.
                //
                // to do .....
                //
            }

        } else if (action.compare("stop") == 0) {                                          // STOP
            std::cout << "stop analyze thread" << std::endl;


            if (!task.LoadTask(task_name, "ITF")) {
                std::cout << "load task fail" << std::endl;

                return -1;
            }

            // Check the current task status from database.
            CTask<float>::TaskStatus_t TaskStatus = static_cast<CTask<float>::TaskStatus_t>(task.getTaskStatus());
            // Check the current function status
            CTask<float>::FuncStatus_t FuncStatus = static_cast<CTask<float>::FuncStatus_t>(task.getFuncStatus());

            if (TaskStatus == CTask<float>::START && FuncStatus == CTask<float>::RUNNING) {

                // Set the current task status as RUNNING
                task.setFuncStatus(CTask<float>::TERMINAL);

                if (t1.joinable())
                    t1.join();

                task.setTaskStatus(CTask<float>::STOP);

                // Update task status(CTask::STOP) to database.
                //
                // to do .....
                //

            }


        } else if (action.compare("EXIT") == 0) {
            std::cout << "stop analyze process" << std::endl;

            if (!task.LoadTask(task_name, "ITF")) {
                std::cout << "load task fail" << std::endl;

                return -1;
            }

            // Check the current task status from database.
            CTask<float>::TaskStatus_t TaskStatus = static_cast<CTask<float>::TaskStatus_t>(task.getTaskStatus());
            // Check the current function status
            CTask<float>::FuncStatus_t FuncStatus = static_cast<CTask<float>::FuncStatus_t>(task.getFuncStatus());

            if (TaskStatus == CTask<float>::START && FuncStatus == CTask<float>::RUNNING) {

                // Since the current analyze process will be removed,
                // any states of process do not need to be preserved.

                // Set the current task status as RUNNING
                task.setFuncStatus(CTask<float>::TERMINAL);

                if (t1.joinable())
                    t1.join();

                break;
            }

        } else {



        }

    }

    std::cout << "ad is done" << std::endl;
    return 0;
}

void analyze() {

    // Init analyzer
    if (!task.InitAnalyzer()) {
        std::cout << "init analyzer fail" << std::endl;
        
        return;
    }

    cv::Mat frame(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();

    std::cout << "analyze" << std::endl;

    CBuffer buffer(task.getCurrentTaskName());
    std::cout << "init buffer " << endl;
    buffer.init(imgSize);
    std::cout << "init buffer done" << endl;
    itf::Util util;

    while (task.getFuncStatus()) {
        // task.getFuncStatus() == RUNNING

        if (!buffer.fetch_src(frame))
            continue;
        vector<float> feature = task.Analyze(frame);

        cv::Mat output(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_32F, feature.data());

        // Put output mat into buffer.
        //
        // to do.
        //

        /*
            Note:
            For different kinds of analyze, there are different post-processing

            1. density: needs to call Util::GenerateHeatMap(cv::Mat pMap) and get sum;
            2. segment: needs to set threshhold on probaility map;
            3. stationary: no post-processing
            4. group: some strange post-processing which I cannot figure out.
        */

        // 1. Density:
        //cv::Mat pMap = cv::Mat::ones(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_32F);
        //cv::Mat heat = util.GenerateHeatMap(output, pMap);
        //cv::imshow("heat", heat);
        //cv::waitKey(1);

        // 2. Segment:
        // cv::Mat foreground = output > 0.5;
        // cv::imshow("foreground", foreground);
        // cv::waitKey(1);
    }
}
