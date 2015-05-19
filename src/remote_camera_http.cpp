//
// ITF RemoteCameraHttp Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "remote_camera_http.hpp"


CRemoteCameraHttp::CRemoteCameraHttp(const std::string &p_host, const std::string &p_port, const std::string &p_path) : 
    CRemoteCamera("http", p_host, p_port, p_path) 
{
    
    
    
}


CRemoteCameraHttp::~CRemoteCameraHttp() {
    
    
    
}


int CRemoteCameraHttp::Connect() {
    
    
    return 0;
}


int CRemoteCameraHttp::Disconnect() {
    
    
    return 0;
}


int CRemoteCameraHttp::Capture() {
    
    
    return 0;
}