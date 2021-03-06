//
// ITF RemoteCameraHttp Class, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#include "remote_camera_http.hpp"

CRemoteCameraHttp::CRemoteCameraHttp(const std::string &p_host, unsigned int p_port, const std::string &p_path, const std::string& p_username, const std::string& p_password) :
    CRemoteCamera("http", p_host, p_port, p_path, p_username, p_password) {
}

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
    return 1;
}

time_t CRemoteCameraHttp::Capture(cv::Mat& output) {
    cap_ >> output;
    time(&rawtime_);
    return rawtime_;
}
