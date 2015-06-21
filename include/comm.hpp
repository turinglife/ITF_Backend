//
// ITF Communication Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_COMM_H
#define ITF_COMM_H

#include <string>

#include "common.hpp"

class CComm {
 public:
    ~CComm();
    void establish(const std::string &socket_name);
    void connect();
    void send();
    void receive(std::string &);

 private:
    std::string socket_path_;
    int sockfd_;
    int newsockfd_;
};


#endif  // ITF_COMM_H
