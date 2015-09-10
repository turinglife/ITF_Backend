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



//#include <iostream>
//#include <string>
#include <thread>
#include "comm.hpp"

#include "task.hpp"


int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);
    CHECK(!task_name.empty()) << "task_name cannot be empty";

    CTask<float> task;
    
    // Initialize trainer
    if (!task.InitTrainer(task_name)) {
        std::cerr << "init trainer fail" << std::endl;
        std::cerr << "rd exit" << std::endl;
        return -1;
    } else {
        std::cout << "Trainer is initialized" << std::endl;
    }
    
#if 1
    // if current type of task is segmentation, it is not necessary to generate a regression model.
    if (task.getCurrentTaskType() == CTask<float>::SEGMENTATION) {
        // write log: segmetation does not need to generate a regression model.
        std::cout << "segmetation does not need to generate a regression model." << std::endl;
        
        return -1;
    }

    // Server comm;
    CComm server;
    std::string socket_path = "RD_" + task_name;
    if (!server.Establish(socket_path)) {
        std::cerr << "Fail to establish connection" << std::endl;
        return -1;
    }

    std::thread t_work;

    std::cout << "rd is ready" << std::endl;
    //int fps = 30; 

    while (true) {
        std::string action;
        server.Receive(action);

        if (action.compare("START") == 0) {  // START
            // capture gt images from foler
            //task.setCameraStatus(CTask<float>::TERMINATE);
            //if (t_work.joinable())
            //    t_work.join();

            //task.setCameraStatus(CTask<float>::RUNNING);
            //t_work = std::thread(&CTask<float>::Capture, &task, fps);
            //task.Capture(fps);
            
            // analyze gt images
            //task.setFuncStatus(CTask<float>::TERMINATE);
            //if (t_work.joinable())
            //    t_work.join();

            task.setFuncStatus(CTask<float>::RUNNING);
            task.setTaskStatus(CTask<float>::ON);

            // Start analyze thread
            //t_work = std::thread(&CTask<float>::Analyze, &task);
            //task.Analyze();
            
            std::string lm_name = "lm";
            task.Train(lm_name);
            
            server.Reply(lm_name);
                
        } else if (action.compare("STOP") == 0) {  // STOP
            task.setCameraStatus(CTask<float>::TERMINATE);
            if (t_work.joinable())
                t_work.join();
            server.Reply("OK");
            break;
        } else {
            std::cerr << "No such command in rd!" << std::endl;
            server.Reply("NO");
        }
    }

    // only unlink after this process ends
    unlink(socket_path.c_str());

    std::cout << "rd is done" << std::endl;
    return 0;
    
#endif
    
}