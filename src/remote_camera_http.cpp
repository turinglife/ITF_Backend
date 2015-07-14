//
// ITF RemoteCameraHttp Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "remote_camera_http.hpp"

#include <string>

CRemoteCameraHttp::CRemoteCameraHttp(const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password) :
    CRemoteCamera("http", p_host, p_port, p_path, p_username, p_password) {
}

CRemoteCameraHttp::~CRemoteCameraHttp() { }

int CRemoteCameraHttp::Connect() {
    // example: http://root:xgwangpj@137.189.35.204:10184/mjpg/video.mjpg
    cap_.open(protocol_ + "://" + username_ + ":" + password_  + "@" + host_ + ":" + std::to_string(port_) + "/" + path_);

    if (cap_.isOpened())
        return 1;
    else
        return 0;
}

int CRemoteCameraHttp::Disconnect() {
    cap_.release();
    return 0;
}

int CRemoteCameraHttp::Capture(cv::Mat& output) {
    if (!cap_.isOpened()) {
        std::cout << "cap_ is not opened" << std::endl;
        return 0;
    }
    cap_ >> output;
    return 1;
}
