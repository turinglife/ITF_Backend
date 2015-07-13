//
// Copyright 2015 CUHK
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/wait.h>

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "dbi.hpp"

bool action_open();
bool action_socket();
bool action_quit();
bool action_mysql();

int main(int argc, char *argv[]) {
    typedef bool(*StringFunc)();
    std::map<std::string, StringFunc> stringToFuncMap;

    stringToFuncMap.insert(std::make_pair("open", &action_open));
    stringToFuncMap.insert(std::make_pair("socket", &action_socket));
    stringToFuncMap.insert(std::make_pair("quit", &action_quit));
    stringToFuncMap.insert(std::make_pair("mysql", &action_mysql));

    bool on = true;
    while (on) {
        // suspend for one second
        sleep(1);

        std::cout << "Action: ";
        std::string action;
        std::cin >> action;

        on = stringToFuncMap[action]();
    }
    return 0;
}

bool action_open() {
    std::cout << "--- action_open() ---" << std::endl;

    std::cout << "Task Name: ";
    std::string task_name;
    std::cin >> task_name;

    std::cout << "Daemon Type: ";
    std::string daemon;
    std::cin >> daemon;

    int status = 0;

    pid_t mpid = fork();
    switch (mpid) {
        case -1:
            std::cout << "Create ad/cd Fail!" << std::endl;
            break;

        case 0: {
            pid_t pid = fork();
            if (pid == 0) {
                std::string path = "./build/daemon/" + daemon + ".bin";
                std::string arg0 = daemon + ".bin_" + task_name;
                std::string arg1 =  task_name;
                char* parmList[] = {const_cast<char*>(arg0.c_str()), const_cast<char*>(arg1.c_str()), 0};
                execv(path.c_str(), parmList);
                std::cout << "ERROR when execv" << std::endl;
            } else {
                exit(0);
            }
        } break;

        default:
            waitpid(mpid, &status, 0);
    }

    return true;
}

bool action_socket() {
    std::cout << "--- action_socket() ---" << std::endl;

    std::cout << "Task_name: ";
    std::string task_name;
    std::cin >> task_name;

    std::cout << "Daemon Type: ";
    std::string daemon;
    std::cin >> daemon;

    std::cout << "operation: ";
    std::string operation;
    std::cin >> operation;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR socket");
        return false;
    }

    std::string socket_path = daemon + "_" + task_name;

    struct sockaddr_un td_addr;
    td_addr.sun_family = AF_UNIX;
    // strcpy(td_addr.sun_path, task_name.c_str());
    snprintf(td_addr.sun_path, socket_path.length() + 1, "%s", socket_path.c_str());

    if (connect(sockfd, (struct sockaddr *) &td_addr, sizeof(td_addr)) < 0) {
        perror("ERROR connect");
        return true;
    }

    if (write(sockfd, operation.c_str(), operation.length() + 1) < 0) {
        perror("ERROR write");
        return false;
    }

    close(sockfd);

    return true;
}

bool action_quit() {
    std::cout << "--- action_quit() ---" << std::endl;

    return false;
}

bool action_mysql() {
    /* Prepare DB Infomation*/
    const std::string server = "localhost";
    const std::string user = "itf";
    const std::string pass = "itf";
    const std::string db_name = "ITF";

    CDbi db;

    std::cout << "\nConnect  ...  ";
    if (!db.Connect(server, user, pass)) {
        std::cout << "Fail" << std::endl;
    } else {
        std::cout << "OK" << std::endl;
    }

     std::cout << "Select DB  ...  ";
    if (!db.UseDB(db_name)) {
        std::cout << "Fail" << std::endl;
    } else {
        std::cout << "OK" << std::endl;
    }
    {
        bool ok = db.RunSQL("INSERT INTO Tasks VALUES ('task_six', 'DENSITY', 'HTTP', 720, 576, 'mjpg/video.mjpg', 'ITFcs1.0/tasks/task_one/', 'PMap/', 'ROI/', 'LM/', 'GT/', 'Alarm/', 'pers.csv', 'roi.csv', 'lm.csv', 'ON', 'ON', 'OFF', 'mmlab');");
        if (!ok) {
            std::cerr << "INSERT DB ... Fail" << std::endl;
        } else {
            std::cout << "INSERT DB ... OK" << std::endl;
        }
    }
    { 
        bool ok = db.RunSQL("DELETE FROM Tasks WHERE task_name='task_six';");
        if (!ok) {
            std::cerr << "DELETE DB ... Fail" << std::endl;
        } else {
            std::cout << "DELETE DB ... OK" << std::endl;
        }
    }
    {
        bool ok = db.RunSQL("UPDATE Tasks SET width="+std::to_string(rand() % 100)+" WHERE task_name='task_six';");
        if (!ok) {
            std::cerr << "UPDATE DB ... Fail" << std::endl;
        } else {
            std::cout << "UPDATE DB ... OK" << std::endl;
        }
    }
    std::cout << "SELECT DB ... \n";
    std::vector<std::map<std::string, std::string> > res = db.Query("SELECT * FROM Tasks;");

    std::cout.setf(std::ios::left);
    for (size_t i = 0; i < res.size(); ++i) {
        std::cout << std::setw(18) << res[i]["task_name"] << std::setw(18) << res[i]["task_type"]
         << std::setw(5) << res[i]["camera_type"] << std::setw(5) << res[i]["width"]
         << std::setw(5) << res[i]["height"] << std::setw(22) << res[i]["address"]
         << std::setw(40) << res[i]["task_path"] << std::setw(8) << res[i]["pers_dir"]
         << std::setw(8) << res[i]["lm_dir"] << std::setw(8) << res[i]["gt_dir"] << std::setw(8) << res[i]["roi_dir"]
         << std::setw(8) << res[i]["alarm_dir"] << std::setw(11) << res[i]["pers_file"]
         << std::setw(11) << res[i]["roi_file"] << std::setw(11) << res[i]["lm_file"]
         << std::setw(5) << res[i]["alarm_switch"] << std::setw(5) << res[i]["report_switch"]
         << std::setw(5) << res[i]["lm_switch"] << std::setw(11) << res[i]["group_name"];

        std::cout << std::endl;
    }

    return true;
}
