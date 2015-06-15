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

bool action_open();
bool action_socket();
bool action_quit();

int main(int argc, char *argv[]) {
    typedef bool(*StringFunc)();
    std::map<std::string, StringFunc> stringToFuncMap;

    stringToFuncMap.insert(std::make_pair("open", &action_open));
    stringToFuncMap.insert(std::make_pair("socket", &action_socket));
    stringToFuncMap.insert(std::make_pair("quit", &action_quit));

    bool on = true;
    while (on) {
        // suspend for one second
        usleep(1000 * 1000);

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
