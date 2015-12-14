//
// Copyright 2015 CUHK
//

#include "comm.hpp"
#include "task.hpp"

// kFPS may needs to be read from database
const int kFPS = 25;

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  LOG_IF(ERROR, argc < 2) << "task_name is missing!";
  std::string task_name(argv[1]);
  CTask<float> task;
  // initialize camera
  if (!task.InitCapture(task_name))
    LOG(ERROR) << "initialze capture fail!";
  // setup server
  std::string socket_path = "CD_" + task_name;
  CComm server;
  if (!server.Establish(socket_path))
    LOG(ERROR) << "Fail to establish connection";

  std::thread worker;
  while (true) {
    std::string action;
    server.Receive(action);
    if (action.compare("START") == 0) {
      task.setCameraStatus(CTask<float>::TERMINATE);
      if (worker.joinable()) worker.join();
      task.setCameraStatus(CTask<float>::RUNNING);
      worker = std::thread(&CTask<float>::Capture, &task, kFPS);
      server.Reply("OK");
    } else if (action.compare("STOP") == 0) {
      task.setCameraStatus(CTask<float>::TERMINATE);
      if (worker.joinable()) worker.join();
      server.Reply("OK");
      break;
    } else {
      LOG(WARNING) << action << " is an invalid command!";
      server.Reply("NO");
    }
  }
  task.FreeBuffer();
  // only unlink after this process ends
  unlink(socket_path.c_str());
  std::cout << task_name << ": cd exits successfully!" << std::endl;
  return 0;
}
