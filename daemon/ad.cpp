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

#include <itf/extracters/extracter_factory.hpp>
#include <itf/segmenters/segmenter_factory.hpp>
#include <itf/util/Util.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "task.hpp"
#include "comm.hpp"
#include "SQLiteCpp/SQLiteCpp.h"


void density(std::string path);
void segmentation(std::string path);

int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);

    CTask task;
    if  (!task.LoadTask(task_name, "db/ITF.db")) {
        std::cout << "load task fail" << std::endl;
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

        if (action.compare("density") == 0) {
            std::cout << "density()" << std::endl;

            task.on = false;
            if (t1.joinable())
                t1.join();
            task.on = true;
            t1 = task.Analyze(CTask::FunType_t::COUNT);
        }

        if (action.compare("segmentation") == 0) {
            std::cout << "segmentation()" << std::endl;

            task.on = false;
            if (t1.joinable())
                t1.join();
            task.on = true;
            t1 = task.Analyze(CTask::FunType_t::SEGMENT);
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
