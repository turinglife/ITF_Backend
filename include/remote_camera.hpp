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

protected:
    std::string protocol;
    std::string host;
    std::string port;
    std::string path;
    std::string username;
    std::string password;

public:
    CRemoteCamera(const std::string &p_protocol, const std::string &p_host, const std::string &p_port, const std::string &p_path);
    virtual ~CRemoteCamera();
    
    virtual int Connect() = 0;
    virtual int Disconnect() = 0;
    virtual int Capture() = 0;            

};





#endif // ITF_RC_H
