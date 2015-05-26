//
// ITF RemoteCamera Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "remote_camera.hpp"


CRemoteCamera::CRemoteCamera(const std::string &p_protocol, const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password) : 
    CCamera(),
    protocol_(p_protocol),
    host_(p_host),
    port_(p_port),
    path_(p_path),
    username_(p_username),
    password_(p_password)
{
    
    
    
}


CRemoteCamera::~CRemoteCamera() {
    
    
    
}