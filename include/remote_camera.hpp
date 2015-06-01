//
// ITF Remote Camera Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_RC_H
#define ITF_RC_H


#include "common.hpp"
#include "camera.hpp"

//
// Class representing 'remote' cameras, i.e. those which are
// accessed over a network connection.
//
class CRemoteCamera : public CCamera {
public:
    CRemoteCamera(const std::string &p_protocol, const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password);
    virtual ~CRemoteCamera();
    
    virtual int Connect() = 0;
    virtual int Disconnect() = 0;
    virtual int Capture(cv::Mat& output) = 0;

protected:
    std::string protocol_;
    std::string host_;
    unsigned int port_ = 0;
    std::string path_;
    std::string username_;
    std::string password_;      

};


#endif // ITF_RC_H
