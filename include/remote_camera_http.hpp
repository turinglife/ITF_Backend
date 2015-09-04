//
// ITF Remote Camera HTTP Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_RCHTTP_H
#define ITF_RCHTTP_H

#include "common.hpp"
#include "remote_camera.hpp"

//
// Class representing 'http' cameras, i.e. those which are
// accessed over a network connection using http
//
class CRemoteCameraHttp : public CRemoteCamera {
 public:
    CRemoteCameraHttp(const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password);
    
    int Connect();                                      
    int Disconnect();
    time_t Capture(cv::Mat& output);
};

#endif // ITF_RCHTTP_H
