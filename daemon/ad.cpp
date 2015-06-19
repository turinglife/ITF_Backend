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
    
    if(!task.LoadTask(task_name, "db/ITF.db")) {
        std::cout << "load task fail" << std::endl;
        return -1;
    }
    
    if(!task.InitAnalyzer()) {
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

    comm.distroy();

    if (unlink(socket_path.c_str()) == -1) {
        std::cout << "ERROR unlink: " << socket_path << std::endl;
        return -1;
    }
    

    std::cout << "ad is done" << std::endl;
    return 0;
}

void analyze() {
    itf::Util util;    
    
    CBuffer buffer(task.getCurrentTaskName());
        
    cv::Mat frame(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_8UC3);
    int imgSize = frame.total() * frame.elemSize();
    
    buffer.init(imgSize);
    
    while (task.on) {

        buffer.fetch(frame);
 
        vector<float> feature = task.Analyze(frame);

        cv::Mat density_map(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_32F, feature.data());

        std::cout << cv::sum(density_map)[0] << std::endl;

        // Generate a heat map
        //cv::Mat pmap(task.getCurrentFrameHeight(), task.getCurrentFrameWidth(), CV_8UC1);
        //cv::Mat heat = util.GenerateHeatMap(density_map, pmap, 2000);
        //cv::imshow(task.getCurrentTaskName() + "_heat", heat);

        //cv::waitKey(3);
    }

    std::cout << task.getCurrentTaskName() << ": Video is Over" << std::endl;

}
