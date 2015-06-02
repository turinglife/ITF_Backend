//
// Copyright 2015 CUHK
//

#include "file_camera.hpp"

#include <string>

CFileCamera::CFileCamera(const std::string& filename) : filename_(filename) { }

CFileCamera::~CFileCamera() { }

int CFileCamera::Connect() {
    cap_.open(filename_);

    if (cap_.isOpened())
        return 1;
    else
        return 0;
}

int CFileCamera::Disconnect() {
    cap_.release();
    return 1;
}

int CFileCamera::Capture(cv::Mat& output) {
    if (!cap_.isOpened()) {
        std::cout << "cap_ is not opened" << std::endl;
        return 0;
    }
    cap_ >> output;
    return 1;
}
