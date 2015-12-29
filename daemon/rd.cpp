//
// Copyright 2015 CUHK
//

#include "comm.hpp"
#include "task.hpp"


int main(int argc, char* argv[]) {
    std::string task_name(argv[1]);
    CHECK(!task_name.empty()) << "task_name cannot be empty";
    
    google::InitGoogleLogging(argv[0]);
    LOG_IF(ERROR, argc < 2) << "task_name is missing!";
    
    std::string home_path(std::getenv("HOME"));
    std::string log_path = home_path + "/ITF_SmartClient/log/";
  
    //google::SetLogDestination(google::GLOG_INFO, log_path.c_str());
    //google::SetLogDestination(google::GLOG_WARNING, log_path.c_str());
    //google::SetLogDestination(google::GLOG_ERROR, log_path.c_str());
    //google::SetLogDestination(google::GLOG_FATAL, log_path.c_str());
    
    // FLAGS_logtostderr = 0: log to file
    FLAGS_logtostderr = 1;

    CTask<float> task;
    
    // Initialize trainer
    if (!task.InitTrainer(task_name)) {
        std::cerr << "init trainer fail" << std::endl;
        std::cerr << "rd exit" << std::endl;
        return false;
    } else {
        std::cout << "Trainer is initialized" << std::endl;
    }
    
        // establish connection with client.
    CComm server;
    std::string socket_path = "RD_" + task_name;
    if (!server.Establish(socket_path)) {
        std::cerr << "Fail to establish connection" << std::endl;
        return false;
    }

    // if current type of task is segmentation, it is not necessary to generate a regression model.
    if (task.getCurrentTaskType() == CTask<float>::SEGMENTATION) {
        // write log: segmetation does not need to generate a regression model.
        std::cout << "segmetation does not need to generate a regression model." << std::endl;
        
        return false;
    }

    std::thread worker;
    std::cout << "rd is ready" << std::endl;
    while (true) {
        std::string action;
        server.Receive(action);

        if (action.compare("START") == 0) {  // START
            task.setTrainerStatus(CTask<float>::TERMINATE);

            std::string lm_name = "lm";
            //task.Train(lm_name);            
            
            if (worker.joinable()) 
                worker.join();
            task.setTrainerStatus(CTask<float>::RUNNING);
            LOG(INFO) <<"start to create Train thread";
            worker = std::thread(&CTask<float>::Train, &task, lm_name);
            
            // to ensure that the procedure of training linear model is completed before sending message to client.
            if (worker.joinable()) 
                worker.join();
            
            task.setTrainerStatus(CTask<float>::TERMINATE);
            server.Reply("OK");
            
            // only once performed.
            break;
        } else if (action.compare("STOP") == 0) {  // STOP   
            task.setTrainerStatus(CTask<float>::TERMINATE);
            if (worker.joinable()) 
                worker.join();
            server.Reply("OK");
            break;
            
        } else if (action.length() > 1) {
            std::cerr << "No such command in rd!" << std::endl;
            server.Reply("NO");
        }
    }

    // only unlink after this process ends
    unlink(socket_path.c_str());
    
    std::cout << task_name << ": md exits successfully!" << std::endl;
    
    return true;    
}
