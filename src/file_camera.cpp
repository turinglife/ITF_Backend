//
// Copyright 2015 CUHK
//

#include "file_camera.hpp"


CFileCamera::CFileCamera(const std::string& filename) : filename_(filename) { }

int CFileCamera::Connect() {
    cap_.open(filename_);
    if (cap_.isOpened())
        return 1;
    else
        return 0;
    rawtime_ = 0;
}

int CFileCamera::Disconnect() {
    cap_.release();
    return 1;
}

time_t CFileCamera::Capture(cv::Mat& output) {
    cap_ >> output;
    rawtime_ += 1;
    return rawtime_;
}
