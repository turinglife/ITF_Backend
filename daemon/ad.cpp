//
// Copyright 2015 CUHK
//

#include "comm.hpp"
#include "task.hpp"

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    LOG_IF(ERROR, argc < 2) << "task_name is missing!";
    
    std::string home_path(std::getenv("HOME"));
    std::string log_path = home_path + "/ITF_SmartClient/log/";
  
    google::SetLogDestination(google::GLOG_INFO, log_path.c_str());
    google::SetLogDestination(google::GLOG_WARNING, log_path.c_str());
    google::SetLogDestination(google::GLOG_ERROR, log_path.c_str());
    google::SetLogDestination(google::GLOG_FATAL, log_path.c_str());
    
    // log to file
    FLAGS_logtostderr = 0;
    
    std::string task_name(argv[1]);
  
    // establish connection with client.
    std::string socket_path = "AD_" + task_name;
    CComm server;
    if (!server.Establish(socket_path))
        LOG(ERROR) << "Fail to establish connection";
  
    CTask<float> task;
    // initialize analyzer
    if (!task.InitAnalyzer(task_name))
        LOG(ERROR) << "initialze analyzer fail!";
  
    std::thread worker;
    while (true) {
        std::string action;
        server.Receive(action);
        if (action.compare("START") == 0) {
            task.setFuncStatus(CTask<float>::TERMINATE);
            if (worker.joinable()) 
                worker.join();
            task.setFuncStatus(CTask<float>::RUNNING);
            task.setTaskStatus(CTask<float>::ON);
            worker = std::thread(&CTask<float>::Analyze, &task);
            server.Reply("OK");
        } else if (action.compare("STOP") == 0) {
            task.setFuncStatus(CTask<float>::TERMINATE);
            task.setTaskStatus(CTask<float>::OFF);
            if (worker.joinable()) 
                worker.join();
            server.Reply("OK");
            break;
        } else if (action.length() > 1) {
            LOG(WARNING) <<"action length is "<<action.length();
            LOG(WARNING) << action << " is an invalid command!";
            server.Reply("NO");
        } else {

        }
    }
    
    // only unlink after this process ends
    unlink(socket_path.c_str());
    std::cout << task_name << ": ad exits successfully!" << std::endl;
    return 0;
}
