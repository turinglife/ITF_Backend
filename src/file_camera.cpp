
#include "file_camera.hpp"

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

int CFileCamera::Capture() {
    return 1;
}