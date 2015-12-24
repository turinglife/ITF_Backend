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
    
    // init 
    CTask<float> task;
    // initialize alarmer
    if (!task.InitAlarmer(task_name))
        LOG(ERROR) << "initialze alarmer fail!";
    
    // establish connection with client
    std::string socket_path = "MD_" + task_name;
    CComm server;
    if (!server.Establish(socket_path))
        LOG(ERROR) << "Fail to establish connection";
    
    std::thread worker;
    while (true) {
        std::string action;
        server.Receive(action);
            if (action.compare("START") == 0) {            
            task.setAlarmerStatus(CTask<float>::TERMINATE);
            if (worker.joinable()) 
                worker.join();
            task.setAlarmerStatus(CTask<float>::RUNNING);
            LOG(INFO) <<"start to create Alarm thread";
            worker = std::thread(&CTask<float>::Alarm, &task);
            server.Reply("OK");
        } else if (action.compare("STOP") == 0) {
            task.setAlarmerStatus(CTask<float>::TERMINATE);
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
    std::cout << task_name << ": md exits successfully!" << std::endl;

    return true;


}