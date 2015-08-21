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


int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);
    CHECK(!task_name.empty()) << "task_name cannot be empty";

    CTask<float> task;
    // Init analyzer
    if (!task.InitAnalyzer(task_name)) {
        std::cerr << "init analyzer fail" << std::endl;
        std::cerr << "ad exit" << std::endl;
        return -1;
    } else {
        std::cout << "Analyzer is initialized" << std::endl;
    }

    // Server comm;
    CComm server;
    std::string socket_path = "AD_" + task_name;
    if (!server.Establish(socket_path)) {
        std::cerr << "Fail to establish connection" << std::endl;
        return -1;
    }

    std::thread t_work;

    std::cout << "ad is ready" << std::endl;

    while (true) {
        std::string action;
        server.Receive(action);

        if (action.compare("START") == 0) {  // START
                task.setFuncStatus(CTask<float>::TERMINATE);
                if (t_work.joinable())
                    t_work.join();

                task.setFuncStatus(CTask<float>::RUNNING);
                task.setTaskStatus(CTask<float>::ON);

                // Start analyze thread
                server.Reply("OK");
                t_work = std::thread(&CTask<float>::Analyze, &task);

        } else if (action.compare("STOP") == 0) {  // STOP
                task.setFuncStatus(CTask<float>::TERMINATE);
                task.setTaskStatus(CTask<float>::OFF);
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
