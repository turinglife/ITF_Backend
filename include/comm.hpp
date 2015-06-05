//
// ITF Communication Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_COMM_H
#define ITF_COMM_H


#include "common.hpp"


class CComm {

    
public:

    
    void establish(const std::string &);
    void connect();
    void distroy();
    void send();
    void receive(std::string &);

private:
    int sockfd_;
    int newsockfd_;


};





#endif // ITF_COMM_H
