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
  
    google::SetLogDestination(google::GLOG_INFO, log_path.c_str());
    google::SetLogDestination(google::GLOG_WARNING, log_path.c_str());
    google::SetLogDestination(google::GLOG_ERROR, log_path.c_str());
    //google::SetLogDestination(google::GLOG_FATAL, log_path.c_str());
    
    // log to file
    FLAGS_logtostderr = 0;
    
    // establish connection with client
    std::string socket_path = "MD_" + task_name;
    CComm server;
    if (!server.Establish(socket_path))
        LOG(ERROR) << "Fail to establish connection";
    
    // init 
    CTask<float> task;
    // initialize alarmer
    if (!task.InitAlarmer(task_name))
        LOG(ERROR) << "initialze alarmer fail!";

    
    // to do.
    
    return 1;
}