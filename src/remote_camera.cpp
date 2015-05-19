//
// ITF RemoteCamera Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "remote_camera.hpp"


CRemoteCamera::CRemoteCamera(const std::string &p_protocol, const std::string &p_host, const std::string &p_port, const std::string &p_path) : 
    CCamera(),
    protocol(p_protocol),
    host(p_host),
    port(p_port),
    path(p_path)
{
    
    
    
}


CRemoteCamera::~CRemoteCamera() {
    
    
    
}