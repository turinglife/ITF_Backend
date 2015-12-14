//
// Copyright 2015 CUHK
//

#include "comm.hpp"
#include "task.hpp"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  LOG_IF(FATAL, argc < 2) << "task_name is missing!";
  std::string task_name(argv[1]);
  CTask<float> task;
  // initialize analyzer
  if (!task.InitAnalyzer(task_name))
    LOG(FATAL) << "initialze analyzer fail!";
  // setup server
  std::string socket_path = "AD_" + task_name;
  CComm server;
  if (!server.Establish(socket_path))
    LOG(FATAL) << "Fail to establish connection";

  std::thread worker;
  while (true) {
    std::string action;
    server.Receive(action);
    if (action.compare("START") == 0) {
      task.setFuncStatus(CTask<float>::TERMINATE);
      if (worker.joinable()) worker.join();
      task.setFuncStatus(CTask<float>::RUNNING);
      task.setTaskStatus(CTask<float>::ON);
      worker = std::thread(&CTask<float>::Analyze, &task);
      server.Reply("OK");
    } else if (action.compare("STOP") == 0) {
      task.setFuncStatus(CTask<float>::TERMINATE);
      task.setTaskStatus(CTask<float>::OFF);
      if (worker.joinable()) worker.join();
      server.Reply("OK");
      break;
    } else {
      LOG(WARNING) << action << " is an invalid command!";
      server.Reply("NO");
    }
  }
  // only unlink after this process ends
  unlink(socket_path.c_str());
  std::cout << task_name << ": ad exits successfully!" << std::endl;
  return 0;
}
